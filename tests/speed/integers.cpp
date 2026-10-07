// Spec: none - tools/speed: atoi, atol, strtol in bases 10, 16 and 0, and strtoul over varied text,
// 1000 calls each; the sums. -DONLY=n (1-6) runs one of the six alone, for a per-call count.
#include <stdio.h>
#include <stdlib.h>
#ifndef ONLY
#define ONLY 0
#endif
#ifndef N
#define N 1000
#endif
// Within long's range for atoi and atol, whose overflow C leaves undefined.
const char *inRange[] = { "-12345", "42", "0", "  +2147483647", "-2147483648", "7", " 31415", "-1",
                          "1000000", "\t-907" };
const char *decimals[] = { "-12345", "42", "0", "  +2147483647", "-2147483648", "99999999999", "7",
                           " 31415", "-1", "1000000" };
const char *hexes[] = { "7fffABCD", "0x1f", "ff", "0XdeadBEEF", "-0x10", "123456789", "0", "zz" };
const char *anybase[] = { "0777", "0x1F", "12345", "-010", "0", "0xFFFFFFFF", "4294967296" };
const char *unsigneds[] = { "4294967295", "-1", "123456", "  +42", "0x10", "18446744073709551616" };
int main()
{
    long s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0;
    unsigned long s6 = 0;
    int a = 0, b = 3, c = 5, d = 0, e = 0, f = 0;
    for (int i = 0; i < N; i++) {
        if (ONLY == 0 || ONLY == 1) s1 += atoi(inRange[a]);
        if (ONLY == 0 || ONLY == 2) s2 += atol(inRange[b]);
        if (ONLY == 0 || ONLY == 3) s3 += strtol(decimals[c], 0, 10);
        if (ONLY == 0 || ONLY == 4) s4 += strtol(hexes[d], 0, 16);
        if (ONLY == 0 || ONLY == 5) s5 += strtol(anybase[e], 0, 0);
        if (ONLY == 0 || ONLY == 6) s6 += strtoul(unsigneds[f], 0, 0);
        if (++a == 10) a = 0;
        if (++b == 10) b = 0;
        if (++c == 10) c = 0;
        if (++d == 8) d = 0;
        if (++e == 7) e = 0;
        if (++f == 6) f = 0;
    }
    printf("%ld %ld %ld %ld %ld %lu\n", s1, s2, s3, s4, s5, s6);
    return 0;
}
