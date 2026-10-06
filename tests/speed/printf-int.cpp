// Spec: none - tools/speed: sprintf of integers in four conversions, 2000 times; a checksum of the text.
#include <stdio.h>
int main()
{
    char buf[64];
    unsigned sum = 0;
    for (int i = 0; i < 2000; i++) {
        int n = sprintf(buf, "%d %u %x %05ld", i * 7919 - 50000, (unsigned)i * 2654435761u, i * 31, (long)i * 13);
        for (int k = 0; k < n; k++) sum = sum * 31 + (unsigned char)buf[k];
    }
    printf("%u\n", sum);
    return 0;
}
