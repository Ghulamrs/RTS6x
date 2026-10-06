// Spec: ISO C 7.12.6.7 log and F.9.3.7: ln x, within an ulp.

#include "Logarithm.h"

namespace rts6x {

double Logarithm::natural(double x)
{
    double result;
    if (special(x, result)) return result;
    return kernel(x).value();
}

}  // namespace rts6x
