// Spec: ISO C 7.12.6.1 exp - Cody and Waite's reduction with a table of 2^(j/64): e^z = 2^k 2^(j/64)
// e^a e^c, a = z - n ln2Hi/64 exact, c the rest (|c| < 2^-28); e^a - 1 - a by Taylor to a^7
// (truncation below 2^-76 for |a| <= ln2/128), evaluated by Estrin's scheme for a short chain.

#include "ExpKernel.h"
#include "MathBits.h"
#include "MathConstants.h"

namespace rts6x {

int ExpKernel::evaluate(double zh, double zl, double &mh, double &ml)
{
    // Constants named once, as locals: cpp11 keeps a call inside an expression on its stack.
    const double scale = MathConstants::sixtyFourByLn2, shift = 6755399441055744.0;
    const double ln2Hi = MathConstants::ln2By64Hi, ln2Lo = MathConstants::ln2By64Lo;
    // n = nearest(z 64/ln2) by the 1.5 * 2^52 shift: its low word is n, |n| < 2^17.
    union { double d; int w[2]; } shifted;
    shifted.d = zh * scale + shift;
    int n = shifted.w[0];
    double nd = shifted.d - shift;
    // n * ln2By64Hi is exact (17 + 36 bits), and so is the subtraction (Sterbenz).
    double a = zh - nd * ln2Hi;
    double c = zl - nd * ln2Lo;
    double a2 = a * a;
    double q = a2 * ((0.5 + a * 0.16666666666666666) + a2 * (0.041666666666666664 + a * 0.008333333333333333)
               + a2 * a2 * (0.001388888888888889 + a * 0.0001984126984126984));
    // e^c - 1 = c + c^2/2 (c^3/6 < 2^-84), and e^(a + c) - 1 - a = q + (e^c - 1)(1 + a + q).
    double b = q + (c + 0.5 * c * c) * (1.0 + (a + q));
    // a = ah + al, ah a multiple of 2^-33 below 2^-7: 26 bits, so top * ah is exact.
    double ah = (a + 786432.0) - 786432.0;
    int j = n & 63;
    double top = powerTop_[j], rest = powerRest_[j];
    double p = top * ah;
    mh = top + p;
    // top >= |p|: the error of mh is exact (Dekker's fast two-sum).
    ml = (p - (mh - top)) + (rest + (top * ((a - ah) + b) + rest * (a + b)));
    return n >> 6;
}

int ExpKernel::evaluate(const DoubleDouble &z, DoubleDouble &m)
{
    double h, l;
    int k = evaluate(z.hi, z.lo, h, l);
    m = DoubleDouble::quickSum(h, l);
    return k;
}

DoubleDouble ExpKernel::scaled(const DoubleDouble &m, int k)
{
    double f = MathBits::powerOfTwo(k);
    return DoubleDouble(m.hi * f, m.lo * f);
}

}  // namespace rts6x
