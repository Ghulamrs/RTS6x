// Spec: none - the floating helpers against the host's IEEE 754 arithmetic: division of doubles
// and floats over edge values and pseudo-random bit patterns (subnormals, infinities and NaNs
// among them), in-range conversions with 64-bit integers, trunc and round; results as checksums.
#include <stdio.h>
#include <math.h>

namespace {

unsigned long long state = 0x9E3779B97F4A7C15ull;
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

// Any NaN counts as the one quiet NaN: which NaN an operation passes on is not what is tested.
unsigned long long canonical(double d) { return d != d ? 0x7FF8000000000000ull : bitsOf(d); }
unsigned canonical(float f) { return f != f ? 0x7FC00000u : bitsOf(f); }

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
    const double edge[] = { 0.0, -0.0, 1.0, -1.0, 3.0, 0.1, 1e308, 1e-308, 4.9e-324, 2.2250738585072014e-308,
                            1.7976931348623157e308, asDouble(0x7FF0000000000000ull), asDouble(0xFFF0000000000000ull),
                            asDouble(0x7FF8000000000000ull), 123456789.0, 1.0 / 3.0 };
    Sum ddiv, fdiv, conv, rnd;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            volatile double x = edge[i], y = edge[j];
            ddiv.add(canonical(x / y));
            volatile float fx = (float)edge[i], fy = (float)edge[j];
            fdiv.add(canonical(fx / fy));
        }
    for (int i = 0; i < 3000; i++) {
        volatile double x = asDouble(next()), y = asDouble(next());
        ddiv.add(canonical(x / y));
        volatile float fx = asFloat((unsigned)next()), fy = asFloat((unsigned)next());
        fdiv.add(canonical(fx / fy));
        // Values well inside 2^62, positive and negative, both ways through the conversions.
        long long n = (long long)(next() >> 2) * ((i & 1) ? 1 : -1);
        unsigned long long u = next() >> 1;
        volatile double dn = (double)n, du = (double)u;
        volatile float fn = (float)n, fu = (float)u;
        conv.add(bitsOf(dn)); conv.add(bitsOf(du)); conv.add(bitsOf(fn)); conv.add(bitsOf(fu));
        conv.add((unsigned long long)(long long)dn); conv.add((unsigned long long)du);
        conv.add((unsigned long long)(long long)fn); conv.add((unsigned long long)fu);
        volatile double small = asDouble((next() & 0x800FFFFFFFFFFFFFull) | (0x3F0ull + next() % 0x70) << 52);
        rnd.add(bitsOf(trunc(small))); rnd.add(bitsOf(round(small)));
    }
    printf("double /: %016llx\nfloat /: %016llx\nconversions: %016llx\ntrunc, round: %016llx\n",
           ddiv.value(), fdiv.value(), conv.value(), rnd.value());
    volatile double h = 2.5, nh = -2.5, t = 7.9, d1 = 1.0, d3 = 3.0;
    printf("samples: %g %g %g %g %.17g %u %llu\n", round(h), round(nh), trunc(t), trunc(-t), d1 / d3,
           (unsigned)4000000000.0, (unsigned long long)1e19);
    return 0;
}
