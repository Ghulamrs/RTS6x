// Spec: ISO C 7.12.7.4 pow and F.9.4.4, every special case in its order: y = +-0 gives 1 for any
// x; x = +1 gives 1 for any y; a NaN otherwise gives a NaN; then the zeros, the infinities, and
// a negative x with a y that is not an integer (EDOM).

#include "Power.h"
#include "MathBits.h"
#include "MathError.h"

namespace rts6x {

double Power::raise(double x, double y)
{
    unsigned long long ux = MathBits::of(x), uy = MathBits::of(y);
    if (MathBits::isZero(uy) || ux == 0x3FF0000000000000ull) return 1.0;
    if (MathBits::isNaN(ux)) return x;
    if (MathBits::isNaN(uy)) return y;
    bool yNegative = MathBits::negative(uy), xNegative = MathBits::negative(ux);
    Parity py = parity(uy);
    if (MathBits::isInfinite(uy)) {
        unsigned long long ax = ux & MathBits::magnitudeMask();
        if (ax == 0x3FF0000000000000ull) return 1.0;
        // |x| < 1 against |x| > 1: +0 or +inf, the other way round for y = -inf.
        return (ax < 0x3FF0000000000000ull) != yNegative ? 0.0 : MathBits::infinity(false);
    }
    if (MathBits::isZero(ux)) {
        bool sign = xNegative && py == Odd;
        return yNegative ? MathError::overflow(sign) : MathBits::zero(sign);
    }
    if (MathBits::isInfinite(ux)) {
        bool sign = xNegative && py == Odd;
        return yNegative ? MathBits::zero(sign) : MathBits::infinity(sign);
    }
    if (xNegative && py == NotInteger) return MathError::domain();
    return finite(MathBits::absolute(x), y, xNegative && py == Odd);
}

}  // namespace rts6x
