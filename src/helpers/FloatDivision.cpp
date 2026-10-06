// Spec: IEEE 754 6.1-6.2 and 7.2-7.3: x/y with NaN in giving NaN out, inf/inf and 0/0 the default
// NaN, x/0 an infinity, x/inf a zero, the sign the operands' exclusive or; else the exact quotient,
// rounded. The quotient's bits by long division, the remainder's non-zero-ness the sticky bit.

#include "SoftFloat.h"

namespace rts6x {

unsigned long long FloatArithmetic::divide(const FloatFormat &format, unsigned long long a, unsigned long long b)
{
    UnpackedFloat x(a, format), y(b, format);
    bool negative = x.negative() != y.negative();
    if (x.kind() == UnpackedFloat::NotANumber) return quiet(format, a);
    if (y.kind() == UnpackedFloat::NotANumber) return quiet(format, b);
    if (x.kind() == UnpackedFloat::Infinite)
        return y.kind() == UnpackedFloat::Infinite ? format.nan() : format.infinity(negative);
    if (y.kind() == UnpackedFloat::Infinite) return format.zero(negative);
    if (y.kind() == UnpackedFloat::Zero)
        return x.kind() == UnpackedFloat::Zero ? format.nan() : format.infinity(negative);
    if (x.kind() == UnpackedFloat::Zero) return format.zero(negative);

    // Both significands in [2^(p-1), 2^p): k turns give floor(sx / sy x 2^(k-1)), p + 2 bits or more.
    const int turns = format.precision() + 3;
    unsigned long long q = 0, r = x.significand(), d = y.significand();
    for (int i = 0; i < turns; i++) {
        q <<= 1;
        if (r >= d) { r -= d; q |= 1; }
        r <<= 1;
    }
    return FloatPacker::pack(format, negative, x.exponent() - y.exponent() - (turns - 1), q, r != 0);
}

}  // namespace rts6x
