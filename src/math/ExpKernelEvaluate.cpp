// Spec: ISO C 7.12.6.1 exp - the reduction of Cody and Waite with a table of 2^(j/64), and the
// Taylor series of e^r - 1 - r to r^7 (truncation below 2^-75 for |r| <= ln2/128).

#include "ExpKernel.h"
#include "MathBits.h"
#include "MathConstants.h"

namespace rts6x {

int ExpKernel::evaluate(const DoubleDouble &z, DoubleDouble &m)
{
    double t = z.hi * MathConstants::sixtyFourByLn2();
    int n = (int)(t >= 0 ? t + 0.5 : t - 0.5);
    double nd = (double)n;
    // n * ln2By64Hi is exact (17 + 36 bits), and so is the subtraction (Sterbenz).
    double r0 = z.hi - nd * MathConstants::ln2By64Hi();
    DoubleDouble r = DoubleDouble::sum(r0, -nd * MathConstants::ln2By64Lo());
    r = DoubleDouble::quickSum(r.hi, r.lo + z.lo);
    double a = r.hi;
    // e^r - 1 - a, less the part from r.lo: a^2/2 + ... + a^7/5040, and r.lo (1 + a).
    double q = a * a * (0.5 + a * (0.16666666666666666 + a * (0.041666666666666664 + a * (0.008333333333333333
               + a * (0.001388888888888889 + a * 0.0001984126984126984)))));
    double b = r.lo + a * r.lo + q;
    int j = n & 63, k = (n - j) / 64;
    double th = powerHi_[j], tl = powerLo_[j];
    DoubleDouble p = DoubleDouble::product(th, a);
    DoubleDouble s = DoubleDouble::sum(th, p.hi);
    m = DoubleDouble::quickSum(s.hi, s.lo + p.lo + tl + th * b + tl * a);
    return k;
}

DoubleDouble ExpKernel::scaled(const DoubleDouble &m, int k)
{
    double f = MathBits::powerOfTwo(k);
    return DoubleDouble(m.hi * f, m.lo * f);
}

}  // namespace rts6x
