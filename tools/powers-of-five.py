#!/usr/bin/env python3
# Spec: none - writes src/stdlib/PowersOfFive.cpp from Python's exact integers: 5^b for b < 28, and
# for 5^(28a), a in [-13, 11], C = floor(5^(28a) 2^-k) in [2^63, 2^64) with k and whether C is exact.
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'stdlib', 'PowersOfFive.cpp')
STEP, LOW, HIGH = 28, -13, 11

rows = []
for a in range(LOW, HIGH + 1):
    if a >= 0:
        v = 5 ** (STEP * a)
        k = v.bit_length() - 64
        c = v >> k if k >= 0 else v << -k
        exact = (c << k if k >= 0 else c >> -k) == v and (k <= 0 or v % (1 << k) == 0)
    else:
        d = 5 ** (STEP * -a)
        m = 63 + d.bit_length()
        c = (1 << m) // d
        if c.bit_length() > 64:
            m -= 1
            c = (1 << m) // d
        if c.bit_length() < 64:
            m += 1
            c = (1 << m) // d
        k = -m
        exact = False
    assert c.bit_length() == 64
    limbs = [(c >> (32 * i)) & 0xFFFFFFFF for i in range(2)]
    rows.append((a, limbs, k, exact))

with open(OUT, 'w') as f:
    f.write('// Spec: none - written by tools/powers-of-five.py: powers of five for DecimalBinary - 5^b exact for\n')
    f.write('// b < 28, and C = floor(5^(28a) 2^-k) in [2^63, 2^64) for a = -13 .. 11, limbs least first.\n\n')
    f.write('#include "DecimalBinary.h"\n\nnamespace rts6x {\n\n')
    f.write('const unsigned long long DecimalBinary::fives[28] = {\n')
    for b in range(28):
        f.write('    0x%016Xull,%s\n' % (5 ** b, '' if b < 27 else ''))
    f.write('};\n\n')
    f.write('const DecimalBinary::Power DecimalBinary::powers[%d] = {\n' % len(rows))
    for a, limbs, k, exact in rows:
        f.write('    { { 0x%08Xu, 0x%08Xu }, %d, %d },    // 5^%d\n'
                % (limbs[0], limbs[1], k, 1 if exact else 0, STEP * a))
    f.write('};\n\n}  // namespace rts6x\n')
