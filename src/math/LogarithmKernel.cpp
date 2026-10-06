// Spec: ISO C 7.12.6.7 log - the table-driven reduction of Logarithm.h; the terms of ln(1 + u)
// summed in a double-double, u^2/2 exactly, the rest in double.

#include "Logarithm.h"
#include "MathBits.h"
#include "MathConstants.h"

namespace rts6x {

DoubleDouble Logarithm::kernel(double x)
{
    unsigned long long u = MathBits::of(x);
    int e;
    if (MathBits::biased(u) == 0) {
        UnpackedFloat a(u, FloatFormat::binary64());
        e = a.exponent() + 52;
        u = a.significand();
    } else {
        e = MathBits::biased(u) - 1023;
    }
    u = (u & MathBits::fractionMask()) | (1023ull << 52);
    // j: the fraction rounded to 1/128; at 128, m/2 is taken against r_0 = 1.
    int j = (int)((u >> 45) & 0x7F) + (int)((u >> 44) & 1);
    double m = MathBits::from(u);
    if (j == 128) { m *= 0.5; e += 1; j = 0; }
    DoubleDouble p = DoubleDouble::product(m, reciprocal_[j]);
    // p.hi lies in [0.99, 1.01], so p.hi - 1 is exact.
    DoubleDouble w = DoubleDouble::sum(p.hi - 1.0, p.lo);
    double v = w.hi;
    DoubleDouble sq = DoubleDouble::product(v, v);
    double tail = v * v * v * (0.3333333333333333 + v * (-0.25 + v * (0.2 + v * (-0.16666666666666666
                  + v * (0.14285714285714285 + v * (-0.125 + v * 0.1111111111111111))))));
    double ed = (double)e;
    DoubleDouble s = DoubleDouble::sum(ed * MathConstants::ln2Hi(), logHi_[j]);
    DoubleDouble t = DoubleDouble::sum(s.hi, v);
    DoubleDouble h = DoubleDouble::sum(t.hi, -0.5 * sq.hi);
    double lo = s.lo + t.lo + h.lo + ed * MathConstants::ln2Lo() + logLo_[j] + w.lo
                + (tail - 0.5 * sq.lo - v * w.lo);
    return DoubleDouble::quickSum(h.hi, lo);
}

}  // namespace rts6x
