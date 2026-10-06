// Spec: ISO C 7.12.4.5 cos and F.9.1.5: cos(+-0) is 1, cos(+-inf) a domain error; below 2^-27
// cos x rounds to 1 (x^2/2 < 2^-55).

#include "Trigonometric.h"
#include "ArgumentReduction.h"
#include "MathBits.h"
#include "MathError.h"

namespace rts6x {

double Trigonometric::cosine(double x)
{
    unsigned long long u = MathBits::of(x);
    if (MathBits::isNaN(u)) return x;
    if (MathBits::isInfinite(u)) return MathError::domain();
    if (MathBits::below(x, -27)) return 1.0;
    DoubleDouble r;
    switch (ArgumentReduction::reduce(x, r)) {
    case 0: return cosineKernel(r).value();
    case 1: return -sineKernel(r).value();
    case 2: return -cosineKernel(r).value();
    default: return sineKernel(r).value();
    }
}

}  // namespace rts6x
