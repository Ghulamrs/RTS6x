// Spec: none - tools/speed: sin, cos, exp, log, sqrt, pow and atan2, 300 times each; their sum.
#include <stdio.h>
#include <math.h>
int main()
{
    double sum = 0;
    for (int i = 1; i <= 300; i++) {
        double x = i * 0.0137;
        sum += sin(x) + cos(x) + exp(x * 0.1) + log(x) + sqrt(x) + pow(x, 1.7) + atan2(x, 1.5);
    }
    printf("%.9e\n", sum);
    return 0;
}
