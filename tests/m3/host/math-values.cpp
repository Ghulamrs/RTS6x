// Spec: none - one program, two builds: on the host over src/math's classes (-DHOST), on the C6000
// over rts6x.lib's C functions. Prints the bits of 6600 results from a fixed input stream, so
// math-target-check.sh can require the two builds to agree bit for bit.
#include <stdio.h>
#ifdef HOST
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
#define F_sqrt SquareRoot::root
#define F_floor IntegralPart::down
#define F_ceil IntegralPart::up
#define F_exp Exponential::natural
#define F_log Logarithm::natural
#define F_log10 Logarithm::common
#define F_sin Trigonometric::sine
#define F_cos Trigonometric::cosine
#define F_tan Trigonometric::tangent
#define F_atan ArcTangent::arcTangent
#define F_asin ArcTangent::arcSine
#define F_acos ArcTangent::arcCosine
#define F_sinh Hyperbolic::sine
#define F_cosh Hyperbolic::cosine
#define F_tanh Hyperbolic::tangent
#define F_pow Power::raise
#define F_atan2 ArcTangent::arcTangent2
#define F_fmod Remainder::truncated
#define F_ldexp BinaryScale::scale
#else
#include <math.h>
#define F_sqrt sqrt
#define F_floor floor
#define F_ceil ceil
#define F_exp exp
#define F_log log
#define F_log10 log10
#define F_sin sin
#define F_cos cos
#define F_tan tan
#define F_atan atan
#define F_asin asin
#define F_acos acos
#define F_sinh sinh
#define F_cosh cosh
#define F_tanh tanh
#define F_pow pow
#define F_atan2 atan2
#define F_fmod fmod
#define F_ldexp ldexp
#endif
static unsigned long long st = 12345;
static unsigned long long nxt() { st = st * 6364136223846793005ull + 1442695040888963407ull; return st; }
static double bits(unsigned long long u) { union { double d; unsigned long long u; } x; x.u = u; return x.d; }
static unsigned long long ob(double d) { union { double d; unsigned long long u; } x; x.d = d; return x.u; }
// random double with exponent in [lo, hi]
static double r(int lo, int hi) { int e = lo + (int)((nxt() >> 33) % (unsigned)(hi - lo + 1)); unsigned long long f = nxt() >> 12; unsigned long long s = (nxt() >> 63) << 63; return bits(s | ((unsigned long long)(e + 1023) << 52) | f); }
static double rp(int lo, int hi) { return bits(ob(r(lo, hi)) & 0x7FFFFFFFFFFFFFFFull); }
static void out(const char *n, double v) { unsigned long long u = ob(v); printf("%s %08x%08x\n", n, (unsigned)(u >> 32), (unsigned)u); }
int main() {
  for (int i = 0; i < 300; i++) {
    double a = r(-30, 30), b = rp(-1022, 1023), t = r(-5, 60), u = r(-30, 0);
    out("sqrt", F_sqrt(b)); out("floor", F_floor(a)); out("ceil", F_ceil(a));
    out("exp", F_exp(r(-8, 9))); out("log", F_log(b)); out("log10", F_log10(b));
    out("sin", F_sin(t)); out("cos", F_cos(t)); out("tan", F_tan(t)); out("sinbig", F_sin(r(20, 1023)));
    out("atan", F_atan(a)); out("asin", F_asin(u)); out("acos", F_acos(u));
    out("sinh", F_sinh(r(-10, 9))); out("cosh", F_cosh(r(-10, 9))); out("tanh", F_tanh(r(-10, 4)));
    out("pow", F_pow(rp(-10, 10), r(-4, 5))); out("atan2", F_atan2(r(-1022, 1023), r(-1022, 1023)));
    out("fmod", F_fmod(r(-1022, 1023), r(-1022, 1023))); out("ldexp", F_ldexp(r(-1022, 1023), (int)(nxt() >> 60) * 150 - 1100));
    out("expsub", F_exp(-708.0 - (double)(nxt() >> 54) / 1024.0 * 37.0)); out("logsub", F_log(bits(nxt() >> 12)));
  }
  return 0;
}
