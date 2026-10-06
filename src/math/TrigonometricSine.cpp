// Spec: ISO C 7.12.4.6 sin and F.9.1.6: sin(+-0) is +-0, sin(+-inf) a domain error; below 2^-27
// sin x rounds to x itself (x^2/6 < 2^-56).

#include "Trigonometric.h"
#include "ArgumentReduction.h"
#include "MathBits.h"
#include "MathError.h"

namespace rts6x {

double Trigonometric::sine(double x)
{
    unsigned long long u = MathBits::of(x);
    if (MathBits::isNaN(u)) return x;
    if (MathBits::isInfinite(u)) return MathError::domain();
    if (MathBits::below(x, -27)) return x;
    DoubleDouble r;
    switch (ArgumentReduction::reduce(x, r)) {
    case 0: return sineKernel(r).value();
    case 1: return cosineKernel(r).value();
    case 2: return -sineKernel(r).value();
    default: return -cosineKernel(r).value();
    }
}

}  // namespace rts6x
