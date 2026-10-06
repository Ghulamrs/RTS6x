// Spec: ISO C 7.12.4.5 cos and F.9.1.5: cos(+-0) is 1, cos(+-inf) a domain error; below 2^-27
// cos x rounds to 1 (x^2/2 < 2^-55).

#include "Trigonometric.h"
#include "MathBits.h"
#include "MathError.h"

namespace rts6x {

double Trigonometric::cosine(double x)
{
    // 2^-27 <= |x| < 2^13 on the high word alone: the fast reduction and kernel.
    union { double d; unsigned w[2]; } v;
    v.d = x;
    if ((v.w[1] & 0x7FFFFFFFu) - 0x3E400000u < 0x02800000u) return fast(x, 1);
    unsigned long long u = MathBits::of(x);
    if (MathBits::isNaN(u)) return x;
    if (MathBits::isInfinite(u)) return MathError::domain();
    if (MathBits::below(x, -27)) return 1.0;
    // cos x = sin(x + pi/2) and cos(-x) = cos x: the sine's quadrant one on.
    return slow(x, 1);
}

}  // namespace rts6x
