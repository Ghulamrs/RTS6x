// Spec: ISO C 7.20.1.2 and 7.20.1.4 - atoi, atol, strtol, strtoul at their edges: white space, signs,
// the base 0 and 16 prefixes, octal, bases 2 and 36, every way past the range, the end pointer, and
// no digits at all. Each line: the case, the value, where the conversion stopped, and ERANGE or '-'.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// .expected: clang with Microsoft's C library, whose long is 32 bits as here - but for "0x", "0xg"
// and "0x 1" in bases 0 and 16, where that library stops before the 0 and C (and glibc) after it.
static const char *texts[] = {
    "", "   ", "+", "-", "+-1", "- 1", " \t\n\v\f\r42", "  -123xyz", "+0", "-0", "0042", "\xff" "12",
    "12abc", "2147483647", "2147483648", "-2147483648", "-2147483649", "4294967295", "4294967296",
    "-4294967295", "-4294967296", "-1", "0000000000002147483647", "000000000004294967296",
    "00000000000000000000", "2999999999", "3000000000", "3999999999", "4000000000", "4999999999",
    "5000000000", "9999999999", "10000000000", "99999999999999999999", "+4294967295", "-2999999999",
    "0x1A", "0X1a", "0x", "0xg", "0x 1", "010", "08", "0", "-0x10", "  +0777", "fF", "-FFFFFFFF",
    "0xFFFFFFFF", "0x100000000", "0x0000000000ffffffff", "0x7fffffff", "-0x80000000", "-0x80000001",
    "037777777777", "040000000000", "19", "11111111111111111111111111111111",
    "111111111111111111111111111111111", "1z141z3", "1z141z4", "zz", "ZZ", "1:", "9a", "7 8",
};
static const int bases[] = { 10, 0, 16, 8, 2, 36 };

static void line(const char *kind, int i, int base, long value, const char *start, const char *end)
{
    printf("%s %d base %d: %ld stop %d %s\n", kind, i, base, value, (int)(end - start),
           errno == ERANGE ? "ERANGE" : "-");
}

int main()
{
    int count = (int)(sizeof texts / sizeof texts[0]);
    for (int i = 0; i < count; i++) {
        for (int b = 0; b < 6; b++) {
            char *end;
            errno = 0;
            long v = strtol(texts[i], &end, bases[b]);
            line("strtol", i, bases[b], v, texts[i], end);
            errno = 0;
            unsigned long u = strtoul(texts[i], &end, bases[b]);
            line("strtoul", i, bases[b], (long)u, texts[i], end);
        }
        errno = 0;
        long v = strtol(texts[i], 0, 10);  // no end pointer
        printf("strtol-noend %d: %ld %s\n", i, v, errno == ERANGE ? "ERANGE" : "-");
    }
    // atoi and atol within long's range only: past it C leaves them undefined.
    static const int inRange[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 21, 22, 24 };
    for (unsigned k = 0; k < sizeof inRange / sizeof inRange[0]; k++) {
        int i = inRange[k];
        printf("atoi %d: %d atol %ld\n", i, atoi(texts[i]), atol(texts[i]));
    }
    return 0;
}
