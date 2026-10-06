// Spec: none - tools/speed: strtod and atoi over decimal text, 1000 times; the sum of the values read.
#include <stdio.h>
#include <stdlib.h>
int main()
{
    static const char *texts[] = { "3.14159265358979", "-2.5e-7", "123456789.125", "1e300", "0.000123", "42" };
    double sum = 0;
    long whole = 0;
    for (int i = 0; i < 1000; i++) {
        sum += strtod(texts[i % 6], 0);
        whole += atoi("-12345") + i;
    }
    printf("%.6e %ld\n", sum, whole);
    return 0;
}
