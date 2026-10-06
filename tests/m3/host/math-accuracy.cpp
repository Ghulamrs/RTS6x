// Spec: none - a host tool: src/math's classes, compiled for the Mac, against the host's libm over
// millions of inputs per function. Prints, per function, how many results differ from libm's and
// by how many ulps; writes every differing input, and a sample, for math-reference.py to judge.
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include "../../../src/math/ArcTangent.h"
#include "../../../src/math/BinaryScale.h"
#include "../../../src/math/Exponential.h"
#include "../../../src/math/Hyperbolic.h"
#include "../../../src/math/IntegralPart.h"
#include "../../../src/math/Logarithm.h"
#include "../../../src/math/Power.h"
#include "../../../src/math/Remainder.h"
#include "../../../src/math/SquareRoot.h"
#include "../../../src/math/Trigonometric.h"

using namespace rts6x;

static unsigned long long bitsOf(double d) { unsigned long long u; std::memcpy(&u, &d, 8); return u; }
static double ofBits(unsigned long long u) { double d; std::memcpy(&d, &u, 8); return d; }

// Ulps between two doubles of one sign, along the ordered line of doubles; NaNs equal to each other.
static double ulps(double a, double b)
{
    if (a != a || b != b) return (a != a && b != b) ? 0 : 1e30;
    long long ia = (long long)bitsOf(a), ib = (long long)bitsOf(b);
    if (ia < 0) ia = (long long)0x8000000000000000ull - ia;
    if (ib < 0) ib = (long long)0x8000000000000000ull - ib;
    long long d = ia - ib;
    return d < 0 ? (double)-d : (double)d;
}

static std::mt19937_64 rng(20261006), thin(7);
static double uniform(double lo, double hi) { return lo + (hi - lo) * std::uniform_real_distribution<double>(0, 1)(rng); }
// A double of either sign with its exponent uniform in [lo, hi] and a random significand.
static double logUniform(int lo, int hi, bool bothSigns = true)
{
    // Built on the bits, so no host arithmetic flushes a subnormal under --ftz.
    int e = std::uniform_int_distribution<int>(lo, hi)(rng);
    unsigned long long frac = rng() & ((1ull << 52) - 1), sign = bothSigns && (rng() & 1) ? 1ull << 63 : 0;
    if (e >= -1022) return ofBits(sign | ((unsigned long long)(e + 1023) << 52) | frac);
    return ofBits(sign | ((frac | (1ull << 52)) >> (-1022 - e + 1)));
}
static double anyBits() { return ofBits(rng()); }

struct Stats {
    const char *name;
    long n = 0, differ = 0, over1 = 0;
    double worst = 0, worstX = 0, worstY = 0;
};

static FILE *dump, *all;
static long sampleEvery = 200, dumpCap = 30000;

static void record(Stats &s, double ours, double ref, double x, double y = 0)
{
    double d = ulps(ours, ref);
    s.n++;
    if (d != 0) s.differ++;
    if (d >= 1) s.over1++;
    if (d > s.worst) { s.worst = d; s.worstX = x; s.worstY = y; }
    // Every input where the two differ, thinned past dumpCap of them, and a regular sample beside.
    bool pick = d != 0 && (s.differ <= dumpCap || (long)(thin() % (unsigned long long)s.differ) < dumpCap);
    if (dump && (pick || s.n % sampleEvery == 0))
        std::fprintf(dump, "%s %016llx %016llx %016llx\n", s.name, bitsOf(x), bitsOf(y), bitsOf(ours));
    if (all) std::fprintf(all, "%s %016llx %016llx %016llx\n", s.name, bitsOf(x), bitsOf(y), bitsOf(ours));
}

static void report(const Stats &s)
{
    std::printf("%-6s %9ld inputs: %8ld differ from libm, %6ld by 1 ulp or more, most %.0f at (%.17g, %.17g)\n",
                s.name, s.n, s.differ, s.over1, s.worst, s.worstX, s.worstY);
}

static void setFlushToZero()
{
#if defined(__aarch64__)
    unsigned long long fpcr;
    __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
    __asm__ volatile("msr fpcr, %0" : : "r"(fpcr | (1ull << 24)));
#endif
}

typedef double (*Unary)(double);
static void unary(const char *name, Unary ours, Unary ref, long n, double (*gen)())
{
    Stats s; s.name = name;
    for (long i = 0; i < n; i++) { double x = gen(); record(s, ours(x), ref(x), x); }
    report(s);
}
typedef double (*Binary)(double, double);
static void binary(const char *name, Binary ours, Binary ref, long n, void (*gen)(double &, double &))
{
    Stats s; s.name = name;
    for (long i = 0; i < n; i++) { double x, y; gen(x, y); record(s, ours(x, y), ref(x, y), x, y); }
    report(s);
}

static double trigInput()
{
    switch (rng() % 4) {
    case 0: return uniform(-10, 10);
    case 1: return logUniform(-30, 1023);
    case 2: { double k = std::floor(uniform(1, 1e6)); double v = k * 1.5707963267948966; return ofBits(bitsOf(v) + (rng() % 9) - 4); }
    default: return uniform(-1e6, 1e6);
    }
}
static double expInput() { return rng() % 2 ? uniform(-746, 710) : logUniform(-60, 3); }
static double logInput()
{
    switch (rng() % 3) {
    case 0: return ofBits(rng() & 0x7FFFFFFFFFFFFFFFull);
    case 1: return 1.0 + logUniform(-60, -1);
    default: return logUniform(-1074, 1023, false);
    }
}
static double atanInput() { return rng() % 2 ? logUniform(-40, 70) : uniform(-4, 4); }
static double unitInput()
{
    switch (rng() % 3) {
    case 0: return uniform(-1, 1);
    case 1: { double v = 1.0 - logUniform(-53, -1, false); return rng() & 1 ? -v : v; }
    default: return logUniform(-40, -1);
    }
}
static double hypInput()
{
    switch (rng() % 3) {
    case 0: return uniform(-25, 25);
    case 1: return logUniform(-40, 0);
    default: return uniform(-712, 712);
    }
}
static double bitsInput() { double v = anyBits(); return rng() % 4 ? v : logUniform(-20, 60); }
static void powInput(double &x, double &y)
{
    switch (rng() % 5) {
    case 0: x = logUniform(-30, 30, false); y = uniform(-20, 20); break;
    case 1: x = logUniform(-1074, 1023, false); y = uniform(-700, 700) / (std::fabs(std::log(x)) + 1e-300); break;
    case 2: x = 1.0 + logUniform(-52, -10); y = logUniform(0, 62); break;
    case 3: x = -std::floor(uniform(1, 1000)); y = std::floor(uniform(-100, 100)); break;
    default: x = uniform(0, 10); y = uniform(-300, 300); break;
    }
}
static void atan2Input(double &y, double &x)
{
    if (rng() % 2) { y = uniform(-10, 10); x = uniform(-10, 10); }
    else { y = logUniform(-1074, 1023); x = logUniform(-1074, 1023); }
}
static void fmodInput(double &x, double &y) { x = bitsInput(); y = rng() % 2 ? bitsInput() : logUniform(-60, 60); }

static double rFloor(double x) { return std::floor(x); }
static double rCeil(double x) { return std::ceil(x); }
static double rTrunc(double x) { return std::trunc(x); }
static double rRound(double x) { return std::round(x); }
static double rFabs(double x) { return std::fabs(x); }
static double oModf(double x) { double w; return IntegralPart::split(x, &w); }
static double oModfW(double x) { double w; IntegralPart::split(x, &w); return w; }
static double rModf(double x) { double w; return std::modf(x, &w); }
static double rModfW(double x) { double w; std::modf(x, &w); return w; }
static double oFrexp(double x) { int e; return BinaryScale::fraction(x, &e); }
static double rFrexp(double x) { int e; return std::frexp(x, &e); }
static double oFrexpE(double x) { int e; BinaryScale::fraction(x, &e); return e; }
static double rFrexpE(double x) { int e; std::frexp(x, &e); return e; }
static double oLdexp(double x, double y) { return BinaryScale::scale(x, (int)y); }
static double rLdexp(double x, double y) { return std::ldexp(x, (int)y); }
static void ldexpInput(double &x, double &y) { x = bitsInput(); y = std::floor(uniform(-2200, 2200)); }

int main(int argc, char **argv)
{
    long n = 1000000;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--ftz")) setFlushToZero();
        else if (!std::strcmp(argv[i], "--dump") && i + 1 < argc) dump = std::fopen(argv[++i], "w");
        else if (!std::strcmp(argv[i], "--all") && i + 1 < argc) all = std::fopen(argv[++i], "w");
        else n = std::atol(argv[i]);
    }
    unary("sqrt", SquareRoot::root, ::sqrt, n, bitsInput);
    unary("floor", IntegralPart::down, rFloor, n, bitsInput);
    unary("ceil", IntegralPart::up, rCeil, n, bitsInput);
    unary("trunc", IntegralPart::towardZero, rTrunc, n, bitsInput);
    unary("round", IntegralPart::nearestAway, rRound, n, bitsInput);
    unary("fabs", BinaryScale::magnitude, rFabs, n, bitsInput);
    unary("modf", oModf, rModf, n, bitsInput);
    unary("modfw", oModfW, rModfW, n, bitsInput);
    unary("frexp", oFrexp, rFrexp, n, bitsInput);
    unary("frexpe", oFrexpE, rFrexpE, n, bitsInput);
    binary("ldexp", oLdexp, rLdexp, n, ldexpInput);
    binary("fmod", Remainder::truncated, ::fmod, n, fmodInput);
    unary("exp", Exponential::natural, ::exp, n, expInput);
    unary("log", Logarithm::natural, ::log, n, logInput);
    unary("log10", Logarithm::common, ::log10, n, logInput);
    binary("pow", Power::raise, ::pow, n, powInput);
    unary("sin", Trigonometric::sine, ::sin, n, trigInput);
    unary("cos", Trigonometric::cosine, ::cos, n, trigInput);
    unary("tan", Trigonometric::tangent, ::tan, n, trigInput);
    unary("atan", ArcTangent::arcTangent, ::atan, n, atanInput);
    binary("atan2", ArcTangent::arcTangent2, ::atan2, n, atan2Input);
    unary("asin", ArcTangent::arcSine, ::asin, n, unitInput);
    unary("acos", ArcTangent::arcCosine, ::acos, n, unitInput);
    unary("sinh", Hyperbolic::sine, ::sinh, n, hypInput);
    unary("cosh", Hyperbolic::cosine, ::cosh, n, hypInput);
    unary("tanh", Hyperbolic::tangent, ::tanh, n, hypInput);
    if (dump) std::fclose(dump);
    if (all) std::fclose(all);
    return 0;
}
