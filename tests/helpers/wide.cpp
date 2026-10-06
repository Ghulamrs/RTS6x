// Spec: none - 64-bit / and %, signed and unsigned, at their edges and over 3000 pseudo-random
// pairs of mixed widths, as checksums the host prints too.
#include <stdio.h>

namespace {

unsigned long long state = 0x2545F4914F6CDD1Dull;
unsigned long long next()
{
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

class Sum {
public:
    Sum() : value_(0) {}
    void add(unsigned long long v) { value_ = value_ * 1099511628211ull + v; }
    unsigned long long value() const { return value_; }
private:
    unsigned long long value_;
};

}  // namespace

int main()
{
    const long long edge[] = { 0, 1, -1, 2, -2, 7, -7, 1000000007LL, -1000000007LL, 4294967296LL, -4294967296LL,
                               9223372036854775807LL, -9223372036854775807LL - 1, 65536 };
    Sum s, u;
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++) {
            volatile long long x = edge[i], y = edge[j];
            if (y == 0 || (x == -9223372036854775807LL - 1 && y == -1)) continue;
            s.add((unsigned long long)(x / y)); s.add((unsigned long long)(x % y));
            volatile unsigned long long ux = (unsigned long long)x, uy = (unsigned long long)y;
            u.add(ux / uy); u.add(ux % uy);
        }
    for (int i = 0; i < 3000; i++) {
        volatile unsigned long long a = next(), b = next() >> (next() % 63);
        if (b == 0) continue;
        u.add(a / b); u.add(a % b);
        volatile long long x = (long long)a, y = (long long)b * ((i & 1) ? -1 : 1);
        if (y == 0) continue;
        s.add((unsigned long long)(x / y)); s.add((unsigned long long)(x % y));
    }
    volatile long long m7 = -7, two = 2;
    printf("signed: %016llx\nunsigned: %016llx\nsamples: %lld %lld %llu\n", s.value(), u.value(), m7 / two, m7 % two,
           18446744073709551615ull / 10ull);
    return 0;
}
