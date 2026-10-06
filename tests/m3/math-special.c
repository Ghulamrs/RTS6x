/* Spec: none - ISO C 7.12 and Annex F's special cases for every <math.h> function: zeros of
   both signs, infinities, NaNs, the domain and pole errors (EDOM, ERANGE), overflow and
   underflow. A host libm that reports through exceptions alone (math_errhandling without
   MATH_ERRNO, as macOS's) cannot witness errno, so there C's required value is printed. */
#include <stdio.h>
#include <math.h>
#include <errno.h>

union bits { double d; unsigned w[2]; };

static double make(unsigned hi, unsigned lo)
{
    union bits b;
    b.w[1] = hi;
    b.w[0] = lo;
    return b.d;
}

static int hostWithoutErrno(void)
{
#if defined(math_errhandling) && defined(MATH_ERRNO)
    return (math_errhandling & MATH_ERRNO) == 0;
#else
    return 0;
#endif
}

static void show(const char *what, double v, int expectErrno)
{
    union bits b;
    int e = errno;
    b.d = v;
    if (hostWithoutErrno()) e = expectErrno;
    printf("%-22s ", what);
    if (v != v) printf("nan");
    else if ((b.w[1] & 0x7FFFFFFFu) == 0x7FF00000u && b.w[0] == 0) printf("%sinf", b.w[1] >> 31 ? "-" : "");
    else if ((b.w[1] & 0x7FFFFFFFu) == 0 && b.w[0] == 0) printf("%s0", b.w[1] >> 31 ? "-" : "");
    else printf("%.17g", v);
    printf(" errno %s\n", e == EDOM ? "EDOM" : e == ERANGE ? "ERANGE" : e == 0 ? "0" : "other");
    errno = 0;
}

#define CHECK(call, expect) show(#call, call, expect)

int main(void)
{
    double inf = make(0x7FF00000u, 0), nan = make(0x7FF80000u, 0), tiny = make(0, 1);
    double pz = 0.0, nz = make(0x80000000u, 0), hard = 6381956970095103.0;
    int i, e;
    double w, f;
    for (i = 0; i < 797; i++) hard *= 2.0;
    errno = 0;
    CHECK(sqrt(nz), 0); CHECK(sqrt(inf), 0); CHECK(sqrt(-1.0), EDOM); CHECK(sqrt(nan), 0);
    CHECK(sqrt(tiny), 0); CHECK(sqrt(2.0), 0); CHECK(sqrt(1e300), 0);
    CHECK(exp(pz), 0); CHECK(exp(nz), 0); CHECK(exp(-inf), 0); CHECK(exp(inf), 0);
    CHECK(exp(710.0), ERANGE); CHECK(exp(-746.0), ERANGE); CHECK(exp(1.0), 0); CHECK(exp(nan), 0);
    CHECK(exp(709.5), 0); CHECK(exp(-740.0), ERANGE); CHECK(exp(-1e-300), 0);
    CHECK(log(pz), ERANGE); CHECK(log(nz), ERANGE); CHECK(log(-1.0), EDOM); CHECK(log(1.0), 0);
    CHECK(log(inf), 0); CHECK(log(nan), 0); CHECK(log(tiny), 0); CHECK(log(-inf), EDOM);
    CHECK(log10(pz), ERANGE); CHECK(log10(-2.0), EDOM); CHECK(log10(1.0), 0); CHECK(log10(10.0), 0);
    CHECK(log10(1000.0), 0); CHECK(log10(1e-300), 0); CHECK(log10(inf), 0); CHECK(log10(1e22), 0);
    CHECK(pow(nan, pz), 0); CHECK(pow(1.0, nan), 0); CHECK(pow(nan, 1.0), 0); CHECK(pow(2.0, nan), 0);
    CHECK(pow(pz, -3.0), ERANGE); CHECK(pow(nz, -3.0), ERANGE); CHECK(pow(nz, -2.0), ERANGE);
    CHECK(pow(nz, -0.5), ERANGE); CHECK(pow(nz, 3.0), 0); CHECK(pow(nz, 2.0), 0); CHECK(pow(nz, 0.5), 0);
    CHECK(pow(-1.0, inf), 0); CHECK(pow(-1.0, -inf), 0); CHECK(pow(0.5, -inf), 0);
    CHECK(pow(2.0, -inf), 0); CHECK(pow(0.5, inf), 0); CHECK(pow(2.0, inf), 0);
    CHECK(pow(-inf, -3.0), 0); CHECK(pow(-inf, -2.0), 0); CHECK(pow(-inf, 3.0), 0);
    CHECK(pow(-inf, 2.5), 0); CHECK(pow(inf, -1.0), 0); CHECK(pow(inf, 0.5), 0);
    CHECK(pow(-8.0, 1.0 / 3.0), EDOM); CHECK(pow(-2.0, 3.0), 0); CHECK(pow(-2.0, 1e300), ERANGE);
    CHECK(pow(10.0, 309.0), ERANGE); CHECK(pow(10.0, -400.0), ERANGE); CHECK(pow(-10.0, 309.0), ERANGE);
    CHECK(pow(2.0, 0.5), 0); CHECK(pow(2.0, 1023.0), 0); CHECK(pow(2.0, -1074.0), ERANGE);
    CHECK(pow(10.0, 22.0), 0); CHECK(pow(1.0000000000000002, 1e18), 0); CHECK(pow(0.25, -0.5), 0);
    CHECK(sin(nz), 0); CHECK(sin(inf), EDOM); CHECK(sin(nan), 0); CHECK(sin(1e22), 0); CHECK(sin(hard), 0);
    CHECK(cos(nz), 0); CHECK(cos(-inf), EDOM); CHECK(cos(1e22), 0); CHECK(cos(hard), 0);
    CHECK(tan(nz), 0); CHECK(tan(inf), EDOM); CHECK(tan(1e22), 0); CHECK(tan(1.5707963267948966), 0);
    CHECK(asin(nz), 0); CHECK(asin(1.0), 0); CHECK(asin(-1.0), 0); CHECK(asin(1.5), EDOM);
    CHECK(acos(1.0), 0); CHECK(acos(-1.0), 0); CHECK(acos(pz), 0); CHECK(acos(-2.0), EDOM);
    CHECK(atan(nz), 0); CHECK(atan(inf), 0); CHECK(atan(-inf), 0); CHECK(atan(nan), 0); CHECK(atan(1.0), 0);
    CHECK(atan2(pz, -1.0), 0); CHECK(atan2(nz, -1.0), 0); CHECK(atan2(pz, 1.0), 0); CHECK(atan2(nz, 1.0), 0);
    CHECK(atan2(pz, nz), 0); CHECK(atan2(nz, nz), 0); CHECK(atan2(pz, pz), 0); CHECK(atan2(nz, pz), 0);
    CHECK(atan2(1.0, pz), 0); CHECK(atan2(-1.0, nz), 0); CHECK(atan2(inf, 1.0), 0); CHECK(atan2(-inf, 1.0), 0);
    CHECK(atan2(inf, -inf), 0); CHECK(atan2(-inf, inf), 0); CHECK(atan2(1.0, -inf), 0); CHECK(atan2(-1.0, inf), 0);
    CHECK(atan2(nan, 1.0), 0); CHECK(atan2(tiny, -1.0), 0); CHECK(atan2(1e-300, 1e300), ERANGE);
    CHECK(sinh(nz), 0); CHECK(sinh(-inf), 0); CHECK(sinh(711.0), ERANGE); CHECK(sinh(-712.0), ERANGE);
    CHECK(cosh(nz), 0); CHECK(cosh(-inf), 0); CHECK(cosh(-710.0), 0); CHECK(cosh(712.0), ERANGE);
    CHECK(tanh(nz), 0); CHECK(tanh(inf), 0); CHECK(tanh(-inf), 0); CHECK(tanh(22.0), 0); CHECK(tanh(nan), 0);
    CHECK(fmod(pz, 3.0), 0); CHECK(fmod(nz, 3.0), 0); CHECK(fmod(5.0, pz), EDOM); CHECK(fmod(inf, 3.0), EDOM);
    CHECK(fmod(5.0, inf), 0); CHECK(fmod(-7.0, 3.0), 0); CHECK(fmod(7.0, -3.0), 0); CHECK(fmod(nan, 1.0), 0);
    CHECK(fmod(1e300, tiny), 0); CHECK(fmod(-6.0, 3.0), 0); CHECK(fmod(5.5, 1.25), 0);
    CHECK(floor(nz), 0); CHECK(floor(-0.5), 0); CHECK(floor(-tiny), 0); CHECK(floor(inf), 0); CHECK(floor(-2.5), 0);
    CHECK(ceil(nz), 0); CHECK(ceil(-0.5), 0); CHECK(ceil(tiny), 0); CHECK(ceil(-inf), 0); CHECK(ceil(2.5), 0);
    CHECK(fabs(nz), 0); CHECK(fabs(-inf), 0); CHECK(fabs(-tiny), 0);
    CHECK(ldexp(1.0, 1024), ERANGE); CHECK(ldexp(1.0, -1075), ERANGE); CHECK(ldexp(1.0, -1074), 0);
    CHECK(ldexp(3.0, -1075), 0); CHECK(ldexp(nz, 5), 0); CHECK(ldexp(inf, -5), 0); CHECK(ldexp(tiny, 1074), 0);
    f = frexp(tiny, &e); printf("frexp(tiny)            %.17g %d\n", f, e);
    f = frexp(-1024.0, &e); printf("frexp(-1024)           %.17g %d\n", f, e);
    f = frexp(nz, &e); show("frexp(-0)", f, 0);
    f = modf(-3.5, &w); printf("modf(-3.5)             %.17g %.17g\n", f, w);
    f = modf(-4.0, &w); show("modf(-4) fraction", f, 0);
    f = modf(-inf, &w); show("modf(-inf) fraction", f, 0);
    show("modf(-inf) whole", w, 0);
    f = modf(nan, &w); show("modf(nan) whole", w, 0);
    f = modf(nz, &w); show("modf(-0) whole", w, 0);
    return 0;
}
