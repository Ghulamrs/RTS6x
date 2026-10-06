// Spec: ISO C 7.12.7.5 sqrt, F.9.4.5, IEEE 754 5.4.1 - correctly rounded. x = S 2^E, E even: the
// integer root T = floor(sqrt(S 2^52)) is guessed by Newton's method in double and then made exact
// with integers (T^2 <= S 2^52 < (T+1)^2); it rounds up exactly when S 2^52 - T^2 > T.

#include "SquareRoot.h"
#include "MathBits.h"
#include "MathError.h"
#include "NewtonIteration.h"

namespace rts6x {

double SquareRoot::root(double x)
{
    unsigned long long u = MathBits::of(x);
    if (MathBits::isZero(u) || MathBits::isNaN(u)) return x;
    if (MathBits::negative(u)) return MathError::domain();
    if (MathBits::isInfinite(u)) return x;
    UnpackedFloat a(u, FloatFormat::binary64());
    unsigned long long s = a.significand();
    int e = a.exponent();
    if (e & 1) { s <<= 1; e -= 1; }
    // s as a double, built on the bits: below 2^53 as it is, else halved (its last bit is 0).
    double w = s >> 53 ? MathBits::from(((s >> 1) & MathBits::fractionMask()) | (1076ull << 52))
                       : MathBits::from((s & MathBits::fractionMask()) | (1075ull << 52));
    // sqrt(s) 2^26, within a few units of T (near 2^52 to 2^53): its integer part from the bits.
    unsigned long long g = MathBits::of(w * NewtonIteration::inverseSquareRoot(w) * 67108864.0);
    int shift = MathBits::biased(g) - 1075;
    unsigned long long t = (g & MathBits::fractionMask()) | (1ull << 52);
    t = shift >= 0 ? t << shift : t >> -shift;
    // S 2^52 - t^2 is small for a t this close, so its low 64 bits are all of it.
    long long rest = (long long)((s << 52) - t * t);
    while (rest < 0) { rest += (long long)(2 * t - 1); t--; }
    while (rest > (long long)(2 * t)) { rest -= (long long)(2 * t + 1); t++; }
    if (rest > (long long)t) t++;
    // t has 53 bits, or is 2^53; the root is normal whatever x was.
    int biased = (e - 52) / 2 + 52 + 1023;
    if (t >> 53) { t >>= 1; biased++; }
    return MathBits::from((t & MathBits::fractionMask()) | ((unsigned long long)biased << 52));
}

}  // namespace rts6x
