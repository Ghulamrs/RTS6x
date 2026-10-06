// Spec: none - tools/speed: int, unsigned and long long division and remainder by a variable, 2000 times.
#include <stdio.h>
int main(int argc, char **)
{
    int d = 7 + argc;
    unsigned ud = 13u + (unsigned)argc;
    long long ld = 1000003LL + argc;
    long long sum = 0;
    for (int i = 1; i <= 2000; i++) {
        sum += (i * 7919) / d + (i * 7919) % d;
        sum += ((unsigned)i * 2654435761u) / ud + ((unsigned)i * 2654435761u) % ud;
        sum += ((long long)i * 123456789012LL) / ld + ((long long)i * 123456789012LL) % ld;
    }
    printf("%lld\n", sum);
    return 0;
}
