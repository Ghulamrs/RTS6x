// Spec: ISO C 7.12.6.1 exp; W. J. Cody and W. Waite, "Software Manual for the Elementary Functions"
// (1980): e^z = 2^k * 2^(j/64) * e^r with z = (64k + j) ln2/64 + r, |r| <= ln2/128, the product
// ln2/64 * n subtracted in two parts so the first is exact; e^r by its Taylor series to r^7.
#ifndef RTS6X_EXP_KERNEL_H
#define RTS6X_EXP_KERNEL_H

#include "DoubleDouble.h"

namespace rts6x {

class ExpKernel {
public:
    // e^(zh + zl) = (mh + ml) 2^k, k returned, mh in [0.7, 1.5), |ml| < 2^-50, to about 2^-67
    // relative; |zh| <= 746, |zl| <= 2^-40. Plain doubles throughout: no call, no double-double.
    static int evaluate(double zh, double zl, double &mh, double &ml);
    // The same with m normalised as a double-double, |m.lo| <= ulp(m.hi)/2.
    static int evaluate(const DoubleDouble &z, DoubleDouble &m);
    // m * 2^k as a double-double, both parts normal: for |k| below about 950.
    static DoubleDouble scaled(const DoubleDouble &m, int k);

private:
    // 2^(j/64) = powerTop_[j] + powerRest_[j], the top of 26 bits.
    static const double powerTop_[64], powerRest_[64];
};

}  // namespace rts6x

#endif
