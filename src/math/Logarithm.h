// Spec: ISO C 7.12.6.7 log, 7.12.6.8 log10 and F.9.3.7, F.9.3.8. x = 2^e * m, m = (1 + u) / r_j
// with r_j near 1/(1 + j/128) from a table, u = m r_j - 1 exactly (Dekker's product), so
// ln x = e ln2 - ln r_j + ln(1 + u), the last by its Taylor series to u^9 (|u| < 2^-7.9).
#ifndef RTS6X_LOGARITHM_H
#define RTS6X_LOGARITHM_H

#include "DoubleDouble.h"

namespace rts6x {

class Logarithm {
public:
    static double natural(double x);    // log
    static double common(double x);     // log10
    // ln x as a double-double, x finite and above zero (subnormal allowed): about 2^-68 relative.
    static DoubleDouble kernel(double x);

private:
    // True with result set for a zero (pole: ERANGE, -inf), x < 0 (EDOM), +inf or a NaN.
    static bool special(double x, double &result);

    static const double reciprocal_[128], logHi_[128], logLo_[128];
};

}  // namespace rts6x

#endif
