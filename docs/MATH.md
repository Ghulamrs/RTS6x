# `<math.h>` in RTS6x

The 24 functions of `<math.h>` that cpp11's and c90's headers declare - `sqrt sin cos tan
asin acos atan atan2 sinh cosh tanh exp log log10 pow fmod floor ceil fabs frexp ldexp modf`,
and `round` and `trunc` by their plain names (cpp11's header spells them `__c6xabi_nround` and
`__c6xabi_trunc` on the C6000, which `src/helpers` already defines). No header declares a
`float` variant, so there is none. `long double` is `double` on this target.

Written from IEEE 754, ISO C 7.12 and Annex F, and textbook methods named in each file's
`Spec:` line - Dekker's exact sum and product, Cody and Waite's reduction, Payne and Hanek's,
the table methods of Gal and Tang, Newton's method, Taylor series with their truncation
bounds. No other library's source was read. Every constant and table is computed by
`tools/math-tables.py` from first principles (pi by Machin's formula, ln 2 by the atanh
series, the rest by their series, in Python integers to 700 bits); rerun it and the files
come out identical.

## The C674x's arithmetic, and what it decides here

The C674x adds and multiplies `double` in hardware, correctly rounded, but **reads a
subnormal operand as zero and flushes a subnormal result to zero** (SPRUFE8 2.8.8;
sim6747 models it). Division and conversions are software helpers. So:

- every classification (zero, subnormal, NaN, infinity) is done on the bits, never by a
  floating compare, which would call a subnormal zero;
- subnormal inputs are taken apart with `UnpackedFloat` (integers); subnormal results are
  built by `MathBits::compose`, which rounds `(hi + lo) 2^k` once from the bits;
- every kernel works on numbers near 1, so no intermediate is ever subnormal;
- no kernel divides: `NewtonIteration` gives `1/d` and `SquareRoot::inverse` `1/sqrt(w)` with
  multiplications only, and a double-double quotient corrects it with the exact remainder.

## The functions

| function | method | file(s) |
| --- | --- | --- |
| `fabs`, `floor`, `ceil`, `trunc`, `round`, `modf`, `frexp` | the bits: fraction cleared, carried into, or the exponent read | `BinaryScale*`, `IntegralPart*` |
| `ldexp` | the exponent field moved; where the result leaves the normal range, `FloatPacker` rounds once | `BinaryScaleScale.cpp` |
| `fmod` | integer long division of the significands, 11 bits a step: exact | `RemainderTruncated.cpp` |
| `sqrt` | a table line for `1/sqrt(m)` (64 bins, `2^-14`) and two Goldschmidt steps give `g` within an ulp; `m - g^2` exactly (Dekker) says which neighbour is the root; within `2^-13` ulp of a half, the integer root `T = floor(sqrt(S 2^52))` settles it exactly | `SquareRoot*`, `SqrtTable.cpp` |
| `exp` | `e^z = 2^k 2^(j/64) e^a e^c`, `a = z - n ln2Hi/64` exact (36-bit part), `c` the rest; `e^a` by Taylor to `a^7` in Estrin's order; the table as a 26-bit top and a rest, so `top * a`'s head is exact; plain doubles, no call | `ExpKernel*`, `ExponentialNatural.cpp` |
| `log`, `log10` | `x = 2^e m`, `r_j` = `1/(1 + j/128)` to 8 bits so `u = m r_j - 1` is exact from two products (Tang), `-ln r_j` a multiple of `2^-42` plus a rest so `e ln2Hi - ln r_j` is exact; `ln(1+u)` by Taylor to `u^10`, `u^2` by Dekker; `log10` times `1/ln10` as a double-double, so a power of ten gives its exponent exactly | `Logarithm*` |
| `pow` | for `x > 0` normal and `y` normal below `2^64` with a normal result, straight to the kernels; otherwise Annex F's special cases in order; `e^(y ln x)` with `ln x` a double-double and `y ln x` an exact product (about 2^-66 relative), `ExpKernel`, one rounding | `Power*` |
| `sin`, `cos`, `tan` | below `2^13`: one Cody-Waite reduction by pi/256 (`n` the index, `t = x - n pi/256` with pi/256 in four parts), sin and cos of `(n mod 128) pi/256` from a table as 26-bit tops and rests, Taylor for `t`, the addition formulas, in plain doubles. Else, and where a sine's remainder is below `2^-37`: `x = k pi/2 + r` by Cody-Waite with pi/2 in four parts below 2^20 (accepted when `|r| >= 2^-59`), else Payne-Hanek (x's significand times a 192-bit window of 2/pi's 1216 bits), then the double-double kernels on `j/64 + t`; `tan` always this way, a double-double quotient | `ArgumentReduction*`, `Trigonometric*` |
| `atan`, `atan2`, `asin`, `acos` | one kernel: `atan y = atan(j/16) + atan((y - c)/(1 + y c))`, Taylor to `t^11`; above 1, `pi/2 - atan(1/x)`; `asin`, `acos` through `sqrt(1 - x^2)` as a double-double, choosing the quotient below 1. `atan2` of two normal arguments within `2^60` of each other: both scaled, `n <= d` the magnitudes, `atan(n/d) = atan c + atan((n - c d)/(d + c n))` with the quotient formed once - a seed reciprocal, two Newton steps and the exact remainder - Taylor to `t^13` | `ArcTangent*` |
| `sinh`, `cosh`, `tanh` | `e^x +- e^-x` from `ExpKernel` as double-doubles; below 1/16 the Taylor series of sinh and cosh; above 38, `e^x/2` alone | `Hyperbolic*` |

Each C function is one tiny `extern "C"` file (`sin.cpp` ...) handing the work to a class.

## Errors (7.12.1)

`EDOM` and a NaN for a domain error (`sqrt`, `log` of a negative, `asin`/`acos` beyond 1,
`fmod(inf, y)`, `fmod(x, 0)`, `pow` of a negative to a non-integer, trigonometric functions of
an infinity); `ERANGE` and `HUGE_VAL` of the right sign for a pole (`log(0)`, `pow(0, y < 0)`)
and an overflow; `ERANGE` for an underflow to zero, and - this library's choice where C leaves
it to the implementation - for `exp`, `pow` and `atan2` results in the subnormal range.

## Accuracy, measured

`tests/m3/host/math-check.sh N` builds the classes for the host with clang++ (no FMA
contraction, so the arithmetic is the C674x's binary64), compares N inputs per function with
the host libm, and has `math-reference.py` judge every input where they differ and a sample
besides against the true value in 90-digit decimal arithmetic (pi to 410 digits for the
reduction). At N = 20,000,000 (2026-10-06):

| function | max error, ulps | judged |
| --- | --- | --- |
| sqrt, fabs, floor, ceil, trunc, round, modf, frexp, ldexp, fmod | 0 (identical to libm on all 20M; each has one right answer) | - |
| exp | 0.500010 | 134,476 |
| log | 0.500003 | 100,536 |
| log10 | 0.500001 | 100,548 |
| pow | 0.500277 | 123,302 |
| sin | 0.500032 | 219,108 |
| cos | 0.500052 | 220,100 |
| tan | 0.500006 | 292,400 |
| atan | 0.500165 | 233,853 |
| atan2 | 0.500270 | 164,565 |
| asin | 0.500395 | 211,685 |
| acos | 0.500507 | 267,105 |
| sinh | 0.501091 | 126,279 |
| cosh | 0.500018 | 125,712 |
| tanh | 0.502981 | 111,058 |

The same check on Linux (g++ 11.5, glibc's libm as the comparison, so a different set is judged)
before and after the speed work of 2026-10-07 (branch `speed/math`), N = 20,000,000 each:

| function | before | after |
| --- | --- | --- |
| exp | 0.500016 | 0.500005 |
| pow | 0.500139 | 0.500003 |
| atan2 | 0.500223 | 0.500015 |
| cosh | 0.500015 | 0.499998 |
| tanh | 0.501176 | 0.500216 |
| sin, cos, tan, log, log10, asin, acos, atan, sinh | as before: 0.500058, 0.500049, 0.500040, 0.500000, 0.499999, 0.500490, 0.500246, 0.500705, 0.500717 | the same |
| sqrt and the exact functions | identical to libm | identical to libm |

The sin and cos maxima are in the double-double path (huge arguments), which is unchanged; the
6600 results of `math-values.cpp` are bit for bit the host's on the C6000 at -O0 and -O2, with
both `rts6x.lib` and `rts6xd.lib`.

The inputs cover every exponent, subnormals, x near 1 for the logarithms, huge
trigonometric arguments and near multiples of pi/2, and exp's and pow's range edges.
macOS's own tan differs from ours on 34% of these inputs, by up to 3 ulps, while ours
stays within 0.500006 of the true value - the host libm is a comparison, not the referee.

`tests/m3/host/math-target-check.sh` then holds the C6000 build to the host's: 6600 results
of `math-values.cpp` (subnormal inputs and results among them) are bit for bit the same from
cpp11 -O0 and -O2 on sim6747 as from the host build that `math-check.sh` measures.
`FTZ=1 math-check.sh` runs the host build under flush-to-zero as a second check that no
kernel leans on subnormal arithmetic: the only differences are inputs the host's own test
arithmetic flushed, and `atan2`'s tiny quotient, which the C674x divides in software.

## Tests in `make check`

- `tests/m3/math-special.c` (c90): Annex F's special cases for every function with errno;
  macOS's libm reports through exceptions only, so there C's required errno is printed.
- `tests/m3/math-exact.c` (c90): the exact functions over 1000 random bit patterns each,
  checksummed - identical to the host's libm.
- `tests/m3/math-accuracy.cpp` (cpp11): 928 inputs with true values from the referee
  (`math-accuracy-table.h`, written by `make-accuracy-table.py`); every result within an ulp.
- c90's own `lib_math` and `fp_nan_ordering` pass on rts6x.lib, on both legs.

## Speed on the C674x (sim6747, cycles a call, cpp11 -O2)

`tests/speed/math.cpp`'s functions one at a time, 300 calls each, the loop's own cost taken out;
TI is `rts6740_elf_eh.lib` linked with the same program:

| function | before (2026-10-06) | after (2026-10-07) | TI |
| --- | --- | --- | --- |
| sin | 1913 | 713 | 274 |
| cos | 1909 | 706 | 254 |
| exp | 1403 | 507 | 514 |
| log | 1639 | 618 | 693 |
| sqrt | 1246 | 480 | 258 |
| pow | 3590 | 1291 | 1045 |
| atan2 | 7224 | 1173 | 997 |
| `tests/speed/math.cpp` | 5,707,421 | 1,654,027 | 1,211,416 |

The rest, from the earlier measurement and on paths not rewritten: floor 113, modf 182, ldexp 226,
frexp 472, log10 2072, sinh 3614, atan 3688, tanh 4193, tan 4742, asin 7422, acos 7621, sin of
1e30 (Payne-Hanek) 14598, fmod(3e8, 0.3) 20898.

What cpp11 -O2 does with this code, measured, and why the kernels are written as they are: it
inlines none of `DoubleDouble`'s members (each sum and product a call returning through memory),
so the fast paths use plain `double`s; it keeps four doubles in saved registers and spills the
rest, while an expression's temporaries stay in registers, so the kernels are long expressions over
few named values; an inline call inside an expression costs a push and a pop of what was evaluated
before it, so `MathConstants` are data members; a 64-bit shift is a generic sequence with branches,
so the bits are read as 32-bit words through a union; and it does not fold constant expressions.
What is left of the gap to TI is the schedule: every product waits out its nine delay slots.
`fmod` with a large exponent gap is bound by the 64-bit remainder helper, and `FloatPacker::pack`
(used where `ldexp` leaves the normal range) by its bit-length loop.
