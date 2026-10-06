// Spec: SPRAB89B 8.2 Table 8-6 (__c6xabi_divull, remull, divlli, remlli: 64-bit / and %, C's
// truncation toward zero, ISO C 6.5.5/6). Restoring division a bit a turn, in 64-bit shifts,
// compares and subtractions only - nothing that would make cpp11 call a division helper itself.
#ifndef RTS6X_WIDE_DIVISION_H
#define RTS6X_WIDE_DIVISION_H

namespace rts6x {

class WideDivision {
public:
    // n / d, with n % d left in remainder; d is never 0 here (C leaves that undefined).
    static unsigned long long divide(unsigned long long n, unsigned long long d, unsigned long long &remainder);
    // The same for signed operands: the quotient truncated toward zero, the remainder with n's sign.
    static long long divide(long long n, long long d, long long &remainder);

private:
    // All ones for a negative value, else zero; (x ^ s) - s negates x where s is all ones.
    static unsigned long long signMask(long long v) { return v < 0 ? ~0ull : 0ull; }
    static unsigned long long negateWhere(unsigned long long x, unsigned long long s) { return (x ^ s) - s; }
};

}  // namespace rts6x

#endif
