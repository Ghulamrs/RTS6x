# Floating division and decimal reading in RTS6x

`__c6xabi_divd`, `__c6xabi_divf` and `strtod` are correctly rounded (D8) and were first written as
exact integer long division and exact BigNumber arithmetic. This file states how the faster
versions stay exact: in each, a fast estimate decides only *how quickly* the answer is found, and
an exact integer check decides *what* it is. Written from IEEE 754 (5.12.2, 7.2-7.3), SPRUFE8 (the
C674x instructions), SPRAB89B (the helpers' registers), Clinger 1990 and Knuth TAOCP vol. 2.

## Division: `src/helpers/divd.s`, `divf.s`

Assembly, an exception to D2: the estimate wants `RCPDP`/`RCPSP` and register pairs that cpp11
cannot reach (it has no intrinsics and keeps eight locals in registers), and the C++ version of the
same method measured 4 to 6 times slower than the assembly. Anything other than two normal operands
with a normal quotient branches, operands untouched, to `FloatArithmetic::divideBinary64/32`, the
general path in C++ (`FloatDivision.cpp`, `SignificandDivision.cpp`).

For x = 1.fx 2^ex and y = 1.fy 2^ey, with significands sx, sy in [2^(p-1), 2^p):

1. **Scale.** Q = sx/sy is in [1, 2) when sx >= sy, else in [1/2, 1). With s = p-1 or p
   respectively, q = floor(sx 2^s / sy) is a p-bit integer and the result's exponent field is
   ex - ey + bias (- 1 when Q < 1). A field outside 1..max-1 goes to the general path.
2. **Guess.** a = 1.fx and b = 1.fy as numbers in [1, 2); `RCPDP`/`RCPSP` give r0 = 1/b to 8 bits,
   so e = 1 - b r0 has |e| < 2^-8. Goldschmidt's products Q0 = a r0, Q_{k+1} = Q_k (1 + e^(2^k))
   give a r0 (1 - e^(2^n)) / (1 - e) = (a/b)(1 - e^(2^n)): three steps in binary64 (|e^8| < 2^-64),
   two in binary32, each rounding once. Its significand m (or the end of the range, when the guess
   lands across 1 from Q) is within a few units of q.
3. **Correct, exactly.** R = sx 2^s - m sy is computed modulo 2^64 (2^32): because |q - m| sy is far
   below 2^63 (2^31), the low word *is* R. While R < 0, m - 1 and R + sy; while R >= sy, m + 1 and
   R - sy. Then m = q and 0 <= R < sy exactly - whatever the guess was, as long as it was close
   enough for the modular step, which the 2^-50 accuracy guarantees with a margin of 2^10.
4. **Round.** Q 2^s = q + R/sy, so the result rounds up when 2R > sy, or 2R = sy and q is odd
   (ties to even). The significand with its hidden bit is added to (field - 1) << fraction bits,
   so a carry to 2^p moves into the field, and from the top field gives infinity.

Checked by `tests/helpers/division-random.cpp` against the host's IEEE division, bit for bit; 2
million double and 2 million float divisions of every class agreed on 2026-10-07 (ROUNDS=2000).

## Reading a decimal: `src/stdlib/FloatTextQuick.cpp`, `FloatTextShort.cpp`, `DecimalBinary.cpp`

`strtod` first tries `FloatText::quick`, which reads white space, a sign, at most 19 significant
digits (w < 10^19 < 2^64) with or without a point, and an exponent, giving w x 10^e. Anything else -
hexadecimal, INF, NAN, more digits - goes to `FloatText::read`, whose decimal path tries the same two
short ways before the exact BigNumber division.

1. **Clinger's way** (`shortWay64`). If w < 2^53 and |e| <= 22, both w and 10^|e| are exact
   binary64 values, so one IEEE multiplication or division gives the correctly rounded w x 10^e.
   For e > 22, tens move into w while it stays below 2^53 ("1e30" is 10^8 x 10^22).
2. **Powers of five** (`DecimalBinary`). w x 10^e = w 5^b 5^(28a) 2^e with e = 28a + b, 0 <= b < 28.
   X = w 5^b is exact in 128 bits; 5^(28a) is held as C = floor(5^(28a) 2^-k) in [2^63, 2^64)
   (`tools/powers-of-five.py`; exact only for a = 0). P = X C, 192 bits, is exact, and the true
   X 5^(28a) 2^-k lies in [P, P + X). Since P >= X 2^63, X is under two units of the last place
   of S, P's top 64 bits. So the true top bits are S, S + 1 or S + 2: the rounding (53 of S's bits,
   then a round bit and the rest) can only change when S's last 11 bits are 0x3FE, 0x3FF, 0x7FE or
   0x7FF, and then the short way declines. Otherwise S rounds as the true value does - with the
   rest counted non-zero when C is inexact, the true value then lying strictly above P. Results
   outside the normal range, or in the top binade, are declined too.
3. **The long way**, as before: the digits' integer as a BigNumber, multiplied out or divided
   bit by bit, exact for every input.

Checked by `tests/m3/strtod-random.cpp` against the host's correctly rounded `strtod`: random
decimals of 1 to 60 digits at every exponent, and exact halfway points between doubles written in
full, cut short and nudged. 320,000 strings (16 seeds, ROUNDS=40, COUNT=500) agreed with clang's
build on the Mac, value, end and ERANGE, on 2026-10-07.

## Running the comparisons large

Both tests are generators: `-DROUNDS=n` (and for strtod `-DCOUNT=n -DSEED=s`) make them as large as
wanted. Build the same file for the host (`clang++ -O1 -ffp-contract=off`) and with cpp11 for the
C6000, run the second on vm6747sim, and compare the printed lines: each is a hash of a block's
results, so a difference names the block.

## Measured, vm6747sim's cycles (cpp11 -O2, `tests/speed`)

| benchmark | before | now | TI rts6740 |
| --- | --- | --- | --- |
| `fdivide` (2000 double and float divisions) | 17,976,166 | 579,167 | 861,134 |
| `strtod` (1000 strtod and 1000 atoi) | 32,461,500 | 2,055,692 | 901,001 |
| the same without `atoi` | - | 1,097,349 | 767,045 |

One `atoi("-12345")` costs about 960 cycles against TI's 134, which is most of what is left of
`strtod`'s ratio; it is `NumberText`'s, not this file's.
