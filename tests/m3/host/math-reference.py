#!/usr/bin/env python3
# Spec: none - the referee for math-accuracy.cpp's dump: each function evaluated in Python's decimal
# arithmetic at 90 digits (pi by Machin's formula; sin, cos and atan by their series), and the
# error of src/math's result measured in ulps of the true value. Prints the largest per function.
import math
import struct
import sys
from decimal import Decimal, getcontext, localcontext

getcontext().prec = 90


def pi_digits(digits):
    """pi to `digits` decimal digits, Machin: pi/4 = 4 atan(1/5) - atan(1/239), in integers."""
    scale = 10 ** (digits + 10)

    def atan_inv(n):
        term, total, k, sign = scale // n, 0, 1, 1
        while term:
            total += sign * (term // k)
            term //= n * n
            k += 2
            sign = -sign
        return total
    return Decimal(4 * (4 * atan_inv(5) - atan_inv(239))) / Decimal(scale)


with localcontext() as c:
    c.prec = 420
    PI_BIG = pi_digits(410)
PI = +PI_BIG


def f(h):
    return struct.unpack('<d', struct.pack('<Q', int(h, 16)))[0]


def sin_cos(x):
    """sin and cos of a Decimal: reduced by pi to 420 digits, then the Taylor series."""
    with localcontext() as c:
        c.prec = 420
        n = (x / PI_BIG).to_integral_value()
        r = x - n * PI_BIG
    odd = int(n) % 2 == 1
    r = +r
    s, co, term, k = Decimal(0), Decimal(0), Decimal(1), 0
    while True:
        if k % 4 == 0:
            co += term
        elif k % 4 == 1:
            s += term
        elif k % 4 == 2:
            co -= term
        else:
            s -= term
        k += 1
        term = term * r / k
        if term == 0 or abs(term) < Decimal(10) ** -120:
            break
    return (-s, -co) if odd else (s, co)


def atan(x):
    if x < 0:
        return -atan(-x)
    if x > 1:
        return PI / 2 - atan(1 / x)
    for _ in range(3):
        x = x / (1 + (1 + x * x).sqrt())
    total, term, k, x2 = Decimal(0), x, 1, x * x
    sign = 1
    while term != 0 and abs(term) > abs(x) * Decimal(10) ** -95:
        total += sign * term / k
        term *= x2
        k += 2
        sign = -sign
    return 8 * total


def exact(name, x, y):
    X, Y = Decimal(x), Decimal(y)
    if name == 'exp':
        return X.exp()
    if name == 'log':
        return X.ln()
    if name == 'log10':
        return X.log10()
    if name == 'sin':
        return sin_cos(X)[0]
    if name == 'cos':
        return sin_cos(X)[1]
    if name == 'tan':
        s, c = sin_cos(X)
        return s / c
    if name == 'atan':
        return atan(X)
    if name == 'asin':
        return atan(X / (1 - X * X).sqrt()) if abs(X) < 1 else (PI / 2 if X > 0 else -PI / 2)
    if name == 'acos':
        if abs(X) == 1:
            return Decimal(0) if X > 0 else PI
        return PI / 2 - atan(X / (1 - X * X).sqrt())
    if name == 'atan2':
        # atan2(x, y) in this file's order: x the ordinate, y the abscissa.
        if y == 0:
            return PI / 2 if x > 0 else -PI / 2
        t = atan(X / Y)
        if y > 0:
            return t
        return t + PI if math.copysign(1.0, x) > 0 else t - PI
    if name == 'sinh':
        return (X.exp() - (-X).exp()) / 2
    if name == 'cosh':
        return (X.exp() + (-X).exp()) / 2
    if name == 'tanh':
        e = (2 * X).exp()
        return (e - 1) / (e + 1)
    if name == 'pow':
        neg = x < 0 and Y == Y.to_integral_value() and int(Y) % 2 == 1
        v = (abs(X).ln() * Y).exp()
        return -v if neg else v
    return None


def ulp_error(ours, ref):
    """|ours - ref| in units of the last place of ref as a double (subnormal ulp 2^-1074)."""
    if ref == 0:
        return 0.0 if ours == 0 else float('inf')
    r = float(ref)
    if math.isinf(r):
        return 0.0 if math.isinf(ours) and (ours > 0) == (ref > 0) else float('inf')
    e = max(math.frexp(abs(r))[1] - 53, -1074)
    if math.isinf(ours):
        return float('inf')
    return float(abs(Decimal(ours) - ref) / (Decimal(2) ** e))


NAMES = ('exp', 'log', 'log10', 'pow', 'sin', 'cos', 'tan', 'atan', 'atan2', 'asin', 'acos',
         'sinh', 'cosh', 'tanh')


def judge(lines):
    """The worst error among lines of one function: (error, x, y, ours, true, count)."""
    getcontext().prec = 90
    worst, count = (-1.0, 0, 0, 0, 0), 0
    for line in lines:
        name, hx, hy, ho = line.split()
        x, y, ours = f(hx), f(hy), f(ho)
        if any(map(math.isnan, (x, y, ours))) or math.isinf(x) or math.isinf(y):
            continue
        try:
            ref = exact(name, x, y)
        except Exception:
            continue
        if ref is None:
            continue
        err = ulp_error(ours, ref)
        count += 1
        if err > worst[0]:
            worst = (err, x, y, ours, float(ref))
    return name, worst, count


def main():
    from multiprocessing import Pool
    chunks = {}
    for line in open(sys.argv[1]):
        name = line.split(' ', 1)[0]
        if name in NAMES:
            chunks.setdefault(name, []).append(line)
    jobs = []
    for name, lines in chunks.items():
        for i in range(0, len(lines), 5000):
            jobs.append(lines[i:i + 5000])
    worst, count = {}, {}
    with Pool() as pool:
        for name, w, n in pool.imap_unordered(judge, jobs):
            count[name] = count.get(name, 0) + n
            if w[0] > worst.get(name, (-1.0,))[0]:
                worst[name] = w
    for name in sorted(worst):
        e, x, y, o, r = worst[name]
        print('%-6s %7d checked: max %.6f ulp at x=%r y=%r (ours %r, true %r)' % (name, count[name], e, x, y, o, r))


if __name__ == '__main__':
    main()
