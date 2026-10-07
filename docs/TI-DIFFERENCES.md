# Where RTS6x and TI's rts6740 print differently, and why RTS6x's answer is right

RTS6x promises exact decimal conversion (D8): `%f`, `%e` and `%g` print the digits of the
double's own value - every binary fraction is a finite decimal one - rounded to nearest, ties
to even (ISO C 7.21.6.1/13 with Annex F, IEEE 754 5.12.2). TI's rts6740 does not: past about
17 significant digits its digits stop being the value's.

## The finding, 2026-10-07

`tests/speed/printf-float.cpp` printed 997790694 linked with rts6x.lib and 1893867233 linked
with rts6740_elf_eh.lib, both on vm6747sim. A diagnostic copy printing each double's bits beside
its text (the 400 lines of the benchmark) showed:

- the doubles are identical bit for bit under both libraries, so the arithmetic (and
  `__c6xabi_divd`) agree;
- `%g` and `%.10e` agree on all 400 lines;
- `%.6f` differs on 171 lines - exactly those whose text has 16 or more significant digits
  (values from 2.6e9 up).

Three lines of it, and the values' exact decimal expansions:

| bits | exact value | RTS6x `%.6f` | rts6740 `%.6f` |
| --- | --- | --- | --- |
| 41fb0df74ee02082 | 7262401774.0079364776611328125 | 7262401774.007936 | 7262401774.007937 |
| 42cb6182a3fe0a47 | 60211235126292.5546875 | 60211235126292.554688 | 60211235126292.550564 |
| 42d2c183ba93a034 | 82489392123520.8125 | 82489392123520.812500 | 82489392123520.815372 |

The second is a tie at the sixth decimal (`...5546875`), rounded to even (`...554688`); the third
is exact in six decimals and rts6740 still invents `...815372`. The same holds for `%e` and `%g`
asked for more than 17 digits: `%.20e` of the first value is `7.26240177400793647766e+09`
(RTS6x) against `7.26240177400793651685e+09` (rts6740).

## The proof

Four independent answers agree with RTS6x on all 400 lines and disagree with rts6740 on the same
171:

1. Python's `decimal` on the Linux box: `Decimal(x)` is the double's exact value, quantized
   with `ROUND_HALF_EVEN` to each format's digits - 0 lines differ from RTS6x.
2. glibc's printf (gcc on Amazon Linux 2023), whose conversion is exact: byte-identical to RTS6x,
   and its checksum of the benchmark is 997790694.
3. MSVC's printf (cl, Visual Studio 2022, on the Windows PC): byte-identical to glibc.
4. The values above worked by hand from their bits.

## What the tree does about it

- `tests/printf/formats.cpp` prints the three values above in `%.6f`, `%.10f`, `%.20e` and
  `%.21g`; `formats.expected` holds the exact answers, so RTS6x is held to them on every run.
  Linked with rts6740 that line comes out wrong.
- `tests/speed/printf-float.expected` holds the host's checksum (997790694), and `tools/speed`
  judges each library's output against a benchmark's `.expected` where there is one, instead of
  against the other library: rts6x must match it, and rts6740 is reported as differing from it
  rather than failing the comparison.
