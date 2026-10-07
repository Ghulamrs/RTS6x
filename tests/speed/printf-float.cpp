// Spec: none - tools/speed: sprintf of doubles in %g, %f and %e, 400 times; a checksum of the text, the host's in printf-float.expected (docs/TI-DIFFERENCES.md).
#include <stdio.h>
int main()
{
    char buf[96];
    unsigned sum = 0;
    double x = 0.1;
    for (int i = 0; i < 400; i++) {
        int n = sprintf(buf, "%g %.6f %.10e", x, x * 1000.0, x / 7.0);
        for (int k = 0; k < n; k++) sum = sum * 31 + (unsigned char)buf[k];
        x = x * 1.37 + 0.5;
        if (x > 1e12) x = x / 1e11;
    }
    printf("%u\n", sum);
    return 0;
}
