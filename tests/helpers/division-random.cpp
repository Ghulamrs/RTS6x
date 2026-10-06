// Spec: none - __c6xabi_divd and __c6xabi_divf against the host's IEEE 754 division, bit for bit:
// random bit patterns, normal operands at every exponent distance, results at the edges of the
// normal range, and quotients built to lie next to a representable value or a halfway point.
#include <stdio.h>

#ifndef ROUNDS
#define ROUNDS 4
#endif

namespace {

unsigned long long state = 0x2545F4914F6CDD1Dull;
unsigned long long next()
{
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

double asDouble(unsigned long long u) { union { double d; unsigned long long u; } x; x.u = u; return x.d; }
unsigned long long bitsOf(double d) { union { double d; unsigned long long u; } x; x.d = d; return x.u; }
float asFloat(unsigned u) { union { float f; unsigned u; } x; x.u = u; return x.f; }
unsigned bitsOf(float f) { union { float f; unsigned u; } x; x.f = f; return x.u; }
unsigned long long canonical(double d) { return d != d ? 0x7FF8000000000000ull : bitsOf(d); }
unsigned canonical(float f) { return f != f ? 0x7FC00000u : bitsOf(f); }

// A double of the given biased exponent, a random fraction and sign.
double normal(unsigned e) { return asDouble((next() & 0x800FFFFFFFFFFFFFull) | ((unsigned long long)e << 52)); }
float single(unsigned e) { return asFloat(((unsigned)next() & 0x807FFFFFu) | (e << 23)); }

class Sum {
public:
    Sum() : value_(0) {}
    void add(unsigned long long v) { value_ = (value_ ^ v) * 1099511628211ull; }
    unsigned long long value() const { return value_; }
private:
    unsigned long long value_;
};

}  // namespace

int main()
{
    for (int round = 0; round < ROUNDS; round++) {
        Sum d, f;
        for (int i = 0; i < 1000; i++) {
            unsigned pick = (unsigned)(next() >> 40);
            double x, y;
            float a, b;
            switch (pick % 6) {
            case 0:     // any bits: zeros, subnormals, infinities and NaNs among them
                x = asDouble(next()); y = asDouble(next());
                a = asFloat((unsigned)next()); b = asFloat((unsigned)next());
                break;
            case 1:     // both normal, exponents near each other
                x = normal(1023 + (pick >> 8) % 64 - 32); y = normal(1023 + (pick >> 16) % 64 - 32);
                a = single(127 + (pick >> 8) % 32 - 16); b = single(127 + (pick >> 16) % 32 - 16);
                break;
            case 2:     // results at and past the ends of the normal range, either way
                x = normal(1 + (pick >> 8) % 2046); y = normal(1 + (pick >> 19) % 2046);
                a = single(1 + (pick >> 8) % 254); b = single(1 + (pick >> 16) % 254);
                break;
            case 3:     // q y rounded: x / y lies at or next to the representable q
                y = normal(1000 + (pick >> 8) % 48); x = normal(1023) * y;
                b = single(110 + (pick >> 8) % 32); a = single(127) * b;
                break;
            case 4: {   // q a halfway point between two values, q y rounded: x / y next to it
                double q = normal(1023), half = (asDouble(bitsOf(q) + 1) - q) * 0.5;
                y = normal(1000 + (pick >> 8) % 48); x = q * y + half * y;
                float r = single(127), s = (asFloat(bitsOf(r) + 1) - r) * 0.5f;
                b = single(110 + (pick >> 8) % 32); a = r * b + s * b;
                break;
            }
            default:    // small integers and their neighbours: exact quotients
                x = asDouble(bitsOf((double)(int)(pick % 1000 + 1)) + (unsigned long long)(long long)((int)((pick >> 20) % 3) - 1));
                y = (double)(int)((pick >> 10) % 100 + 1);
                a = asFloat(bitsOf((float)(int)(pick % 1000 + 1)) + (unsigned)((int)((pick >> 20) % 3) - 1));
                b = (float)(int)((pick >> 10) % 100 + 1);
                break;
            }
            d.add(canonical(x / y));
            f.add(canonical(a / b));
        }
        printf("%d %016llx %016llx\n", round, d.value(), f.value());
    }
    return 0;
}
