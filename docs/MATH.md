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
vm6747sim models it). Division and conversions are software helpers. So:

- every classification (zero, subnormal, NaN, infinity) is done on the bits, never by a
  floating compare, which would call a subnormal zero;
- subnormal inputs are taken apart with `UnpackedFloat` (integers); subnormal results are
  built by `MathBits::compose`, which rounds `(hi + lo) 2^k` once from the bits;
- every kernel works on numbers near 1, so no intermediate is ever subnormal;
- no kernel divides: `NewtonIteration` gives `1/d` and `1/sqrt(w)` with multiplications
  only, and a double-double quotient corrects it with the exact remainder.

## The functions

| function | method | file(s) |
| --- | --- | --- |
| `fabs`, `floor`, `ceil`, `trunc`, `round`, `modf`, `frexp` | the bits: fraction cleared, carried into, or the exponent read | `BinaryScale*`, `IntegralPart*` |
| `ldexp` | the exponent field moved; where the result leaves the normal range, `FloatPacker` rounds once | `BinaryScaleScale.cpp` |
| `fmod` | integer long division of the significands, 11 bits a step: exact | `RemainderTruncated.cpp` |
| `sqrt` | integer root `T = floor(sqrt(S 2^52))`, guessed by Newton in double, made exact in integers, rounded up when `S 2^52 - T^2 > T` | `SquareRootRoot.cpp` |
| `exp` | `e^z = 2^k 2^(j/64) e^r`, `|r| <= ln2/128`, ln2/64 in two parts (the first of 36 bits, so `n ln2/64` is exact); `e^r` by Taylor to `r^7`; the table as double-doubles | `ExpKernel*`, `ExponentialNatural.cpp` |
| `log`, `log10` | `x = 2^e m`, `m r_j = 1 + u` exactly (Dekker), `ln x = e ln2 - ln r_j + ln(1+u)`, Taylor to `u^9`, summed as a double-double; `log10` times `1/ln10` as a double-double, so a power of ten gives its exponent exactly | `Logarithm*` |
| `pow` | Annex F's special cases in order, then `e^(y ln x)` with `ln x` a double-double and `y ln x` an exact product (about 2^-66 relative), `ExpKernel`, one rounding | `Power*` |
| `sin`, `cos`, `tan` | reduction `x = k pi/2 + r`: Cody-Waite with pi/2 in four parts below 2^20 (accepted when `|r| >= 2^-59`, the error being under 2^-129), else Payne-Hanek: x's significand times a 192-bit window of 2/pi's 1216 bits. Then `r = j/64 + t` with sin and cos of j/64 from a table, Taylor for t, the addition formulas; `tan` a double-double quotient | `ArgumentReduction*`, `Trigonometric*` |
| `atan`, `atan2`, `asin`, `acos` | one kernel: `atan y = atan(j/16) + atan((y - c)/(1 + y c))`, Taylor to `t^11`; above 1, `pi/2 - atan(1/x)`; `asin`, `acos` through `sqrt(1 - x^2)` as a double-double, choosing the quotient below 1; `atan2` scales both arguments near one first | `ArcTangent*` |
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

The inputs cover every exponent, subnormals, x near 1 for the logarithms, huge
trigonometric arguments and near multiples of pi/2, and exp's and pow's range edges.
macOS's own tan differs from ours on 34% of these inputs, by up to 3 ulps, while ours
stays within 0.500006 of the true value - the host libm is a comparison, not the referee.

`tests/m3/host/math-target-check.sh` then holds the C6000 build to the host's: 6600 results
of `math-values.cpp` (subnormal inputs and results among them) are bit for bit the same from
cpp11 -O0 and -O2 on vm6747sim as from the host build that `math-check.sh` measures.
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

## Speed on the C674x (vm6747sim, cycles a call, cpp11 -O2)

floor 113, modf 182, ldexp 226, frexp 472, sqrt 1258, exp 1392, cos 1414, sin 1435, log 1654,
log10 2072, pow 3595, sinh 3614, atan 3688, tanh 4193, tan 4742, atan2 7093, asin 7422, acos
7621, sin of 1e30 (Payne-Hanek) 14598, fmod(3e8, 0.3) 20898.

Correct first (D8); what would make it faster, measured: cpp11 inlines none of
`DoubleDouble`'s small members, so a kernel is mostly calls - each sum and product a call
returning through memory. Writing the kernels with plain `double` locals would remove that,
at the cost of the clarity the class gives. `fmod` with a large exponent gap is bound by the
64-bit remainder helper, and `FloatPacker::pack` (used where `ldexp` leaves the normal range)
by its bit-length loop.
