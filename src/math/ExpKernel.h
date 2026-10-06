// Spec: ISO C 7.12.6.1 exp; W. J. Cody and W. Waite, "Software Manual for the Elementary Functions"
// (1980): e^z = 2^k * 2^(j/64) * e^r with z = (64k + j) ln2/64 + r, |r| <= ln2/128, the product
// ln2/64 * n subtracted in two parts so the first is exact; e^r by its Taylor series to r^7.
#ifndef RTS6X_EXP_KERNEL_H
#define RTS6X_EXP_KERNEL_H

#include "DoubleDouble.h"

namespace rts6x {

class ExpKernel {
public:
    // e^(z.hi + z.lo) = m 2^k, k returned, m in [0.7, 1.5) to about 2^-66; |z.hi| <= 746.
    static int evaluate(const DoubleDouble &z, DoubleDouble &m);
    // m * 2^k as a double-double, both parts normal: for |k| below about 950.
    static DoubleDouble scaled(const DoubleDouble &m, int k);

private:
    static const double powerHi_[64], powerLo_[64];
};

}  // namespace rts6x

#endif
