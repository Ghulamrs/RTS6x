// Spec: none - printf, sprintf, fprintf and wprintf over C's conversions, each line compared with
// what the host's own library prints for the same call (clang on the Mac: formats.expected).
#include <stdio.h>

#ifdef __TMS320C6X__
extern "C" int wprintf(const wchar_t *, ...);
#define WIDE(...) wprintf(__VA_ARGS__)
#else
#include <wchar.h>
// The host cannot mix byte and wide output on one stream; the same conversion into a buffer.
#define WIDE(...) do { wchar_t w_[256]; int n_ = swprintf(w_, 256, __VA_ARGS__); \
                       for (int i_ = 0; i_ < n_; i_++) putchar((char)w_[i_]); } while (0)
#endif

// Infinity and NaN from their bits, and thirds as literals, so the test needs no division helper.
static double fromBits(unsigned hi, unsigned lo)
{
    union { double d; unsigned w[2]; } u;
    u.w[0] = lo;
    u.w[1] = hi;
    return u.d;
}

int main()
{
    const double inf = fromBits(0x7FF00000u, 0), nan = fromBits(0x7FF80000u, 0);
    const double third = 0.33333333333333331, twoThirds = 0.66666666666666663;
    int n = 0;
    printf("[%d] [%i] [%5d] [%-5d] [%05d] [%+d] [% d] [%.3d] [%8.3d] [%-+8.3d]\n", 42, -42, 42, 42, -42, 42, 42, 7, -7, 7);
    printf("[%u] [%o] [%#o] [%x] [%#x] [%X] [%#X] [%#.0o] [%.0d] [%#x]\n", 3000000000u, 8, 8, 255, 255, 255, 255, 0, 0, 0);
    printf("[%hhd] [%hd] [%ld] [%lld] [%llu] [%llx] [%jd] [%zu] [%td]\n", 300, 70000, -2147483647L - 1,
           -9223372036854775807LL - 1, 18446744073709551615ULL, 0x123456789abcdefULL, (long long)-5, (size_t)99, 7);
    printf("[%c] [%5c] [%-3c] [%s] [%8s] [%-8s] [%.2s] [%*d] [%-*d] [%.*f]\n", 'A', 'b', 'c', "str", "right", "left", "cut", 6, 1, 6, 2, 3, 3.14159);
    printf("[%f] [%.0f] [%#.0f] [%.10f] [%f] [%f] [%.2f] [%.3f] [%f]\n", 3.14159, 2.5, 3.5, 0.1, -0.0, 1e15, 0.125, 1.0005, 123456789.987654321);
    printf("[%e] [%.0e] [%#.0e] [%E] [%.3e] [%e] [%e] [%.15e]\n", 12345.678, 5.5, 6.5, 0.000123, 9.9995, 1e-300, 1.7976931348623157e308, 0.1);
    printf("[%g] [%g] [%g] [%g] [%g] [%#g] [%.3g] [%G] [%g] [%.10g] [%g]\n", 100000.0, 1000000.0, 0.0001, 0.00001, 1.5, 1.5, 3.14159, 1e-10, 0.0, twoThirds, 123456789.0);
    printf("[%a] [%A] [%a] [%.2a] [%a] [%a]\n", 1.0, 255.5, -0.1, 1.999, 0.0, 4.9406564584124654e-324);
    printf("[%f] [%e] [%g] [%F] [%5.1f] [%-10.3e|] [%010.2f] [%+.1e] [% .2g]\n", inf, -inf, nan, inf, 9.96, 1234.5, -3.14159, 0.0, 12.0);
    printf("[%.0f] [%.0f] [%.0f] [%.1f] [%.1f] [%.20f]\n", 0.5, 1.5, 2.5, 0.25, 0.35, 0.1);
    printf("[%.17g] [%.17g] [%.17g] [%f]\n", 0.1, third, 2.2250738585072014e-308, 1e22);
    // Past 17 significant digits the digits are still the value's own (TI's differ: docs/TI-DIFFERENCES.md).
    const double big1 = fromBits(0x41FB0DF7u, 0x4EE02082u), big2 = fromBits(0x42CB6182u, 0xA3FE0A47u);
    const double big3 = fromBits(0x42D2C183u, 0xBA93A034u);
    printf("[%.6f] [%.6f] [%.6f] [%.10f] [%.20e] [%.21g]\n", big1, big2, big3, big2, big1, big3);
    printf("[%%] [%s] [%n]", (const char *)0, &n);
    printf("%d\n", n);

    char buf[128];
    int k = sprintf(buf, "%s-%04d-%.3f-%x", "id", 7, twoThirds, 48879);
    printf("sprintf %d [%s]\n", k, buf);
    k = sprintf(buf, "%c%c%c", 'x', 'y', 'z');
    printf("sprintf %d [%s]\n", k, buf);

    k = fprintf(stdout, "fprintf [%s] [%d] [%.2e]\n", "to stdout", -123, 6.02214076e23);
    printf("fprintf %d\n", k);

    WIDE(L"wide [%d] [%s] [%ls] [%c] [%lc] [%.3f] [%x]\n", 99, "narrow", L"wide", 'q', L'W', 1.5, 255);
    WIDE(L"wide [%5d|%-5d] [%e]\n", 1, 2, 12345.0);
    return 0;
}
