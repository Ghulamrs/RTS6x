// Spec: none - the transcendental functions of <math.h> against true values computed apart
// (math-accuracy-table.h, from decimal arithmetic): for each, how many results lie within an
// ulp. Large trigonometric arguments, exp's range edges, subnormals and x near 1 are among them.
#include <stdio.h>
#include <math.h>
#include "math-accuracy-table.h"

static unsigned long long bitsOf(double d) { union { double d; unsigned long long u; } b; b.d = d; return b.u; }
static double fromBits(unsigned long long u) { union { double d; unsigned long long u; } b; b.u = u; return b.d; }
// Doubles on one ordered line: the distance between two in ulps.
static long long ordered(unsigned long long u) { return (u >> 63) ? -(long long)(u & 0x7FFFFFFFFFFFFFFFULL) : (long long)u; }

static double call(int function, double x, double y)
{
    switch (function) {
    case 0: return sin(x);
    case 1: return cos(x);
    case 2: return tan(x);
    case 3: return exp(x);
    case 4: return log(x);
    case 5: return log10(x);
    case 6: return pow(x, y);
    case 7: return atan(x);
    case 8: return atan2(x, y);
    case 9: return asin(x);
    case 10: return acos(x);
    case 11: return sinh(x);
    case 12: return cosh(x);
    default: return tanh(x);
    }
}

int main()
{
    static const char *const names[] = { "sin", "cos", "tan", "exp", "log", "log10", "pow", "atan", "atan2",
                                         "asin", "acos", "sinh", "cosh", "tanh" };
    int total[14] = { 0 }, good[14] = { 0 };
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        const Case &c = cases[i];
        double r = call(c.function, fromBits(c.x), fromBits(c.y));
        long long d = ordered(bitsOf(r)) - ordered(c.truth);
        total[c.function]++;
        if (d >= -1 && d <= 1) good[c.function]++;
        else printf("%s(%.17g, %.17g) = %.17g, true %.17g\n", names[c.function], fromBits(c.x), fromBits(c.y), r, fromBits(c.truth));
    }
    for (int f = 0; f < 14; f++) printf("%-6s %d of %d within an ulp\n", names[f], good[f], total[f]);
    return 0;
}
