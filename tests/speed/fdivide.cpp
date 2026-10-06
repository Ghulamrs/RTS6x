// Spec: none - tools/speed: double and float division, 2000 times each; the sums.
#include <stdio.h>
int main()
{
    double s = 0;
    float f = 0;
    for (int i = 1; i <= 2000; i++) {
        s += 1.0 / (i * 0.37 + 1.0);
        f += 1.0f / ((float)i * 0.37f + 1.0f);
    }
    printf("%.12e %.6e\n", s, (double)f);
    return 0;
}
