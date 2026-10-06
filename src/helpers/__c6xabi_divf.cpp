// Spec: SPRAB89B 8.2 Table 8-4 - __c6xabi_divf, x / y in binary32.
// Named steps rather than one expression: cpp11 -O2 stopped on the nested form (2026-10-06).

#include "SoftFloat.h"

extern "C" float __c6xabi_divf(float x, float y)
{
    unsigned long long a = rts6x::FloatBits::of(x), b = rts6x::FloatBits::of(y);
    unsigned long long q = rts6x::FloatArithmetic::divide(rts6x::FloatFormat::binary32(), a, b);
    return rts6x::FloatBits::toFloat(q);
}
