// Spec: Newton's method on f(y) = 1/y^2 - m: the error e becomes about 3e^2/2 at each step, from
// 6.5% for the quadratic 1.5 - 0.525 m + 0.06875 m^2 over m in [1, 4) to below 2^-60 in four.

#include "NewtonIteration.h"
#include "MathBits.h"

namespace rts6x {

double NewtonIteration::inverseSquareRoot(double w)
{
    unsigned long long u = MathBits::of(w);
    int e = MathBits::biased(u) - 1023;
    // w = m 2^e with e even and m in [1, 4).
    int odd = e & 1;
    double m = MathBits::from((u & MathBits::fractionMask()) | ((unsigned long long)(1023 + odd) << 52));
    e -= odd;
    double y = 1.5 + m * (-0.525 + m * 0.06875);
    for (int i = 0; i < 4; i++) y = y * (1.5 - 0.5 * m * y * y);
    return y * MathBits::powerOfTwo(-e / 2);
}

}  // namespace rts6x
