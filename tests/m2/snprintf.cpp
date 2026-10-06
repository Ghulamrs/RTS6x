// Spec: none - snprintf (C99, so not in c90's headers): truncated to n - 1 characters and a NUL,
// nothing at all for n 0, and the count of the whole output returned either way.
#include <stdio.h>

int main()
{
    char buf[16];
    int n = snprintf(buf, 6, "%d-%s", 12345, "xyz");
    printf("%d [%s]\n", n, buf);
    n = snprintf(0, 0, "%s", "counted");
    printf("%d\n", n);
    n = snprintf(buf, 16, "%.3f", 0.66666666666666663);
    printf("%d [%s]\n", n, buf);
    return 0;
}
