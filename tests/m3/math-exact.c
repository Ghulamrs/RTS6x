/* Spec: none - the exact functions of <math.h> (sqrt, floor, ceil, fmod, frexp, ldexp, modf,
   fabs) over 1000 pseudo-random bit patterns each - every exponent, subnormals, infinities and
   NaNs among them - the result bits summed into checksums. Each answer has one right value, so
   the host's libm prints the same lines. */
#include <stdio.h>
#include <math.h>

union bits { double d; unsigned w[2]; };

static unsigned seed = 2026u;
static unsigned next(void) { seed = seed * 1103515245u + 12345u; return (seed >> 8) ^ (seed << 24); }

static double randomDouble(void)
{
    union bits b;
    b.w[0] = next();
    b.w[1] = next();
    return b.d;
}

/* A NaN's payload is not specified, so every NaN counts the same. */
static unsigned mix(unsigned sum, double v)
{
    union bits b;
    b.d = v;
    if (v != v) return sum * 31u + 0x7FF80000u;
    return (sum * 31u + b.w[1]) * 31u + b.w[0];
}

int main(void)
{
    unsigned s[10];
    int i, k, e;
    double x, y, w;
    for (k = 0; k < 10; k++) s[k] = 0;
    for (i = 0; i < 1000; i++) {
        x = randomDouble();
        y = randomDouble();
        if ((i & 3) == 0) y = ldexp(1.0 + (double)(next() & 0xFFFF) / 65536.0, (int)(next() % 120u) - 60);
        s[0] = mix(s[0], sqrt(x));
        s[1] = mix(s[1], floor(x));
        s[2] = mix(s[2], ceil(x));
        s[3] = mix(s[3], fmod(x, y));
        s[4] = mix(s[4], frexp(x, &e));
        s[5] = s[5] * 31u + (unsigned)e;
        s[6] = mix(s[6], ldexp(x, (int)(next() % 4400u) - 2200));
        s[7] = mix(s[7], modf(x, &w));
        s[8] = mix(s[8], w);
        s[9] = mix(s[9], fabs(x));
    }
    printf("sqrt %u\nfloor %u\nceil %u\nfmod %u\nfrexp %u %u\nldexp %u\nmodf %u %u\nfabs %u\n",
           s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9]);
    printf("%.17g %.17g %.17g %.17g\n", sqrt(2.0), sqrt(0.5), sqrt(1e-310), sqrt(1.7976931348623157e308));
    printf("%.17g %.17g %.17g\n", fmod(1e308, 3.0), fmod(-1e-300, 7e-310), fmod(123456789.0, 0.1));
    printf("%.17g %.17g %.17g %.17g\n", floor(4503599627370495.5), ceil(-4503599627370495.5), floor(-1e-310), ceil(1e-310));
    return 0;
}
