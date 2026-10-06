// Spec: SPRAB89B 8.2 Table 8-4 - __c6xabi_divd, x / y in binary64.

#include "SoftFloat.h"

extern "C" double __c6xabi_divd(double x, double y)
{
    return rts6x::FloatBits::toDouble(rts6x::FloatArithmetic::divide(rts6x::FloatFormat::binary64(), rts6x::FloatBits::of(x), rts6x::FloatBits::of(y)));
}
