// Spec: IEEE 754 binary64: sign, 11-bit biased exponent, 52-bit fraction; subnormals, infinities,
// NaNs. Classification and scaling done on the bits, since the C674x reads a subnormal as zero.
#ifndef RTS6X_MATH_BITS_H
#define RTS6X_MATH_BITS_H

#include "../helpers/SoftFloat.h"

namespace rts6x {

class DoubleDouble;

class MathBits {
public:
    static unsigned long long of(double d) { return FloatBits::of(d); }
    static double from(unsigned long long u) { return FloatBits::toDouble(u); }

    static unsigned long long signBit() { return 1ull << 63; }
    static unsigned long long magnitudeMask() { return ~signBit(); }
    static unsigned long long fractionMask() { return (1ull << 52) - 1; }
    static int biased(unsigned long long u) { return (int)((u >> 52) & 0x7FF); }
    static bool negative(unsigned long long u) { return (u >> 63) != 0; }
    static bool isNaN(unsigned long long u) { return (u & magnitudeMask()) > 0x7FF0000000000000ull; }
    static bool isInfinite(unsigned long long u) { return (u & magnitudeMask()) == 0x7FF0000000000000ull; }
    static bool isZero(unsigned long long u) { return (u & magnitudeMask()) == 0; }
    // Finite and not zero: normal or subnormal.
    static bool isFinite(unsigned long long u) { return biased(u) != 0x7FF; }

    static double zero(bool negative) { return from(negative ? signBit() : 0); }
    static double infinity(bool negative) { return from((negative ? signBit() : 0) | 0x7FF0000000000000ull); }
    static double notANumber() { return from(0x7FF8000000000000ull); }
    static double absolute(double x) { return from(of(x) & magnitudeMask()); }
    static double withSign(double magnitude, bool negative)
    {
        return from((of(magnitude) & magnitudeMask()) | (negative ? signBit() : 0));
    }
    // 2^k for -1022 <= k <= 1023.
    static double powerOfTwo(int k) { return from((unsigned long long)(k + 1023) << 52); }
    // |x| below 2^k, x normal or subnormal or zero: compared on the magnitude's bits.
    static bool below(double x, int k) { return (of(x) & magnitudeMask()) < ((unsigned long long)(k + 1023) << 52); }

    // (m.hi + m.lo) * 2^k rounded once, m.hi > 0 and normal, negated if asked; past the largest
    // double, ERANGE and infinity; subnormal or zero, ERANGE.
    static double compose(const DoubleDouble &m, int k, bool negative);
};

}  // namespace rts6x

#endif
