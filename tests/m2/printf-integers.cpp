// Spec: none - the integer half of printf at its edges: INT_MIN and the long long extremes, zero
// under every precision, widths, flags and length modifiers in combination, octal and hex with #,
// snprintf truncating at every size, %n counts and a wide format; printed as the host prints them.
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <stddef.h>

#ifdef __TMS320C6X__
extern "C" int wprintf(const wchar_t *, ...);
#define WIDE(...) wprintf(__VA_ARGS__)
#else
#include <wchar.h>
#define WIDE(...) do { wchar_t w_[64]; int n_ = swprintf(w_, 64, __VA_ARGS__); \
                       for (int i_ = 0; i_ < n_; i_++) putchar((char)w_[i_]); } while (0)
#endif

int main()
{
    printf("[%d] [%i] [%u] [%x] [%o]\n", INT_MIN, INT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
    printf("[%lld] [%lld] [%llu] [%llx] [%llo]\n", LLONG_MIN, LLONG_MAX, ULLONG_MAX, ULLONG_MAX, ULLONG_MAX);
    printf("[%lld] [%llu] [%llX] [%lld]\n", -1234567890123LL, 10000000000ULL, 0xABCDEF0123ULL, 4294967296LL);
    printf("[%d] [%.0d] [%.1d] [%5.0d] [%-5.0d|] [%05d] [%+d] [% d] [%+.0d]\n", 0, 0, 0, 0, 0, 0, 0, 0, 0);
    printf("[%#o] [%#.0o] [%#x] [%#X] [%#.3o] [%#5x] [%#-8x|] [%#08x]\n", 0, 0, 0, 255, 8, 255, 255, 255);
    printf("[%8d] [%-8d|] [%08d] [%+08d] [% 08d] [%-+8d|] [%8.5d] [%-8.5d|] [%08.5d]\n",
           -42, -42, -42, 42, 42, 42, -42, -42, 42);
    printf("[%hhd] [%hhu] [%hd] [%hu] [%ld] [%lu] [%zu] [%td] [%jd]\n", 300, 300, 70000, 70000, -5L, 5UL,
           (size_t)77, (ptrdiff_t)-77, (long long)-9);
    printf("[%*d] [%-*d|] [%.*d] [%*.*d] [%*d|]\n", 6, 12, 6, 12, 4, 12, 7, 3, 5, -6, 9);
    printf("[%x] [%X] [%o] [%u] [%d]\n", 0x89abcdef, 0x89abcdef, 01234567, 4000000000u, -1000000000);
    for (int v = 1; v < 100000000; v = v * 7 + 3) printf("%d %u %x|", v, (unsigned)v * 3u, v);
    printf("\n[%%] [%c%c] [%s] [%.3s] [%8s] [%-8s|]\n", 'o', 'k', "text", "text", "ab", "ab");
    char buf[32];
    for (int size = 0; size < 14; size++) {
        memset(buf, '#', sizeof buf);
        int n = snprintf(buf, (size_t)size, "%d,%x", -123456, 0xbeef);
        printf("%d:%d:%s ", size, n, size ? buf : "-");
    }
    int a = 0, b = 0;
    printf("\n%d%n %s%n|", 12345, &a, "abc", &b);
    printf(" %d %d\n", a, b);
    fflush(stdout);
    WIDE(L"[%d] [%5x] [%-4u|] [%.2d] [%lld]\n", -7, 255, 3u, 4, LLONG_MIN);
    return 0;
}
