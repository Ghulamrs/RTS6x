// Spec: ISO C 6.5.5/6 - integer division truncates toward zero, and (a/b)*b + a%b == a.

#include "WideDivision.h"

namespace rts6x {

unsigned long long WideDivision::divide(unsigned long long n, unsigned long long d, unsigned long long &remainder)
{
    unsigned long long q = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1u);
        q <<= 1;
        if (r >= d) { r -= d; q |= 1u; }
    }
    remainder = r;
    return q;
}

long long WideDivision::divide(long long n, long long d, long long &remainder)
{
    // Branch-free signs: cpp11 left a branch's end label undefined on the C6000 at -O2 (2026-10-06).
    unsigned long long sn = signMask(n), sd = signMask(d), r;
    unsigned long long q = divide(negateWhere((unsigned long long)n, sn), negateWhere((unsigned long long)d, sd), r);
    remainder = (long long)negateWhere(r, sn);
    return (long long)negateWhere(q, sn ^ sd);
}

}  // namespace rts6x
