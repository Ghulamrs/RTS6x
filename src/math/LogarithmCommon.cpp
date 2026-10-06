// Spec: ISO C 7.12.6.8 log10 and F.9.3.8: ln x times 1/ln 10, both double-doubles, rounded once -
// so an exact power of ten gives its exact exponent.

#include "Logarithm.h"
#include "MathConstants.h"

namespace rts6x {

double Logarithm::common(double x)
{
    double result;
    if (special(x, result)) return result;
    DoubleDouble l = kernel(x);
    return l.times(DoubleDouble(MathConstants::invLn10Hi(), MathConstants::invLn10Lo())).value();
}

}  // namespace rts6x
