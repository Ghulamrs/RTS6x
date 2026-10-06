#!/usr/bin/env python3
# Spec: none - writes tests/m3/math-accuracy-table.h: for each transcendental function, inputs
# (random over its domain, and the hard places: huge trigonometric arguments, the edges of exp's
# range, x near 1 for log) with the true value rounded to a double by math-reference.py's
# decimal arithmetic. An input is kept only where the host's libm is itself within an ulp of the
# true value, since the test's .expected is what the host prints.
import importlib.util
import math
import os
import random
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('ref', os.path.join(HERE, 'math-reference.py'))
ref = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ref)

rnd = random.Random(20261006)


def bits(v):
    return struct.unpack('<Q', struct.pack('<d', v))[0]


def ordered(v):
    b = bits(v)
    return (1 << 63) - b if b >> 63 else b


def logu(lo, hi, signed=True):
    v = math.ldexp(1 + rnd.random(), rnd.randint(lo, hi))
    return -v if signed and rnd.random() < 0.5 else v


HARD = 6381956970095103.0 * 2.0 ** 797
INPUTS = {
    'sin': [1e22, HARD, 1e300, 1.7976931348623157e308, 3.141592653589793, 1.5707963267948966, 355.0, 103993.0]
           + [logu(-26, 1023) for _ in range(40)] + [rnd.uniform(-10, 10) for _ in range(20)],
    'cos': [1e22, HARD, 1e300, 3.141592653589793, 1.5707963267948966, 355.0, 103993.0]
           + [logu(-26, 1023) for _ in range(40)] + [rnd.uniform(-10, 10) for _ in range(20)],
    'tan': [1e22, HARD, 1.5707963267948966, 0.7853981633974483, 355.0]
           + [logu(-26, 1023) for _ in range(40)] + [rnd.uniform(-10, 10) for _ in range(20)],
    'exp': [709.78, -708.39, -745.1, -744.0, 1e-10, 0.5, 1.0, -1.0, 100.0]
           + [rnd.uniform(-745, 709) for _ in range(40)] + [logu(-50, 3) for _ in range(20)],
    'log': [5e-324, 2.2250738585072014e-308, 1.7976931348623157e308, 0.9999999999999999, 1.0000000000000002, 2.0, 10.0]
           + [1 + logu(-50, -2) for _ in range(20)] + [logu(-1074, 1023, False) for _ in range(40)],
    'log10': [5e-324, 1.7976931348623157e308, 0.9999999999999999, 1.0000000000000002, 2.0, 1e-300]
             + [1 + logu(-50, -2) for _ in range(20)] + [logu(-1074, 1023, False) for _ in range(40)],
    'atan': [1.0, 1e300, 1e-300, 0.5, 16.0, 1.0000000000000002] + [logu(-30, 70) for _ in range(40)]
            + [rnd.uniform(-3, 3) for _ in range(20)],
    'asin': [0.5, 0.9999999999999999, 1e-10, 0.7071067811865476] + [rnd.uniform(-1, 1) for _ in range(40)]
            + [1 - logu(-52, -2, False) for _ in range(20)],
    'acos': [0.5, 0.9999999999999999, -0.9999999999999999, 1e-10, 0.7071067811865476]
            + [rnd.uniform(-1, 1) for _ in range(40)] + [-(1 - logu(-52, -2, False)) for _ in range(20)],
    'sinh': [1e-5, 0.0625, 0.06249999999999999, 1.0, 38.0, 38.5, 710.0] + [rnd.uniform(-30, 30) for _ in range(40)]
            + [logu(-26, 0) for _ in range(20)],
    'cosh': [1e-5, 0.0625, 1.0, 38.0, 38.5, 710.0] + [rnd.uniform(-30, 30) for _ in range(40)]
            + [logu(-26, 0) for _ in range(20)],
    'tanh': [1e-5, 0.0625, 0.06249999999999999, 1.0, 21.9] + [rnd.uniform(-22, 22) for _ in range(40)]
            + [logu(-26, 0) for _ in range(20)],
}
PAIRS = {
    'pow': [(2.0, 0.5), (10.0, 22.0), (10.0, -5.0), (0.5, 1074.0), (1.0000000000000002, 1e15), (7.0, 1.0 / 3),
            (2.5, -307.5), (-3.0, 7.0), (0.9999999999999999, -1e16)]
           + [(logu(-30, 30, False), rnd.uniform(-20, 20)) for _ in range(40)]
           + [(logu(-500, 500, False), rnd.uniform(-1.5, 1.5)) for _ in range(20)],
    'atan2': [(1.0, -1.0), (-1.0, -1.0), (1e-300, -1.0), (3.0, 4.0), (1e300, 1e-300)]
             + [(rnd.uniform(-10, 10), rnd.uniform(-10, 10)) for _ in range(40)]
             + [(logu(-300, 300), logu(-300, 300)) for _ in range(20)],
}
LIBM = {'sin': math.sin, 'cos': math.cos, 'tan': math.tan, 'exp': math.exp, 'log': math.log, 'log10': math.log10,
        'atan': math.atan, 'asin': math.asin, 'acos': math.acos, 'sinh': math.sinh, 'cosh': math.cosh,
        'tanh': math.tanh, 'pow': math.pow, 'atan2': math.atan2}
ORDER = ['sin', 'cos', 'tan', 'exp', 'log', 'log10', 'pow', 'atan', 'atan2', 'asin', 'acos', 'sinh', 'cosh', 'tanh']

rows = []
for name in ORDER:
    cases = PAIRS[name] if name in PAIRS else [(x, 0.0) for x in INPUTS[name]]
    kept = 0
    for x, y in cases:
        r = ref.exact(name, x, y)
        try:
            true = float(r)
        except OverflowError:
            continue
        if math.isinf(true) or true == 0:
            continue
        host = LIBM[name](x, y) if name in PAIRS else LIBM[name](x)
        if abs(ordered(host) - ordered(true)) > 1:
            continue
        rows.append('    { %d, 0x%016XULL, 0x%016XULL, 0x%016XULL },' % (ORDER.index(name), bits(x), bits(y), bits(true)))
        kept += 1
    print('%-6s %d of %d kept' % (name, kept, len(cases)))
with open(os.path.join(HERE, '..', 'math-accuracy-table.h'), 'w') as f:
    f.write('// Spec: none - generated by tests/m3/host/make-accuracy-table.py: function, x, y, and the true\n'
            '// value rounded to a double, each as its bits.\n')
    f.write('static const struct Case { int function; unsigned long long x, y, truth; } cases[] = {\n')
    f.write('\n'.join(rows) + '\n};\n')
