/* Spec: none - 32-bit / and % at their edges and over 2000 pseudo-random pairs (a 32-bit LCG,
   the same on any host), the results summed into checksums; the host prints the same lines. */
#include <stdio.h>

static unsigned seed = 12345u;
static unsigned next(void) { seed = seed * 1103515245u + 12345u; return seed; }

int main(void)
{
    static const int edge[] = { 0, 1, -1, 2, -2, 7, -7, 1000, -1000, 2147483647, -2147483647 - 1, 65536, -65536 };
    unsigned sq = 0, sr = 0, uq = 0, ur = 0;
    int i, j;
    for (i = 0; i < 13; i++)
        for (j = 0; j < 13; j++) {
            int x = edge[i], y = edge[j];
            if (y == 0 || (x == -2147483647 - 1 && y == -1)) continue;
            sq = sq * 31u + (unsigned)(x / y);
            sr = sr * 31u + (unsigned)(x % y);
            uq = uq * 31u + (unsigned)x / (unsigned)y;
            ur = ur * 31u + (unsigned)x % (unsigned)y;
        }
    printf("edges: %u %u %u %u\n", sq, sr, uq, ur);
    for (i = 0; i < 2000; i++) {
        unsigned a = next(), b = next() >> (next() % 31u);
        int x = (int)a, y = (int)b;
        if (b == 0) continue;
        sq = sq * 31u + (unsigned)(x / y);
        sr = sr * 31u + (unsigned)(x % y);
        uq = uq * 31u + a / b;
        ur = ur * 31u + a % b;
    }
    printf("random: %u %u %u %u\n", sq, sr, uq, ur);
    printf("samples: %d %d %d %d %u %u\n", -7 / 2, -7 % 2, 7 / -2, 7 % -2, 4000000000u / 3u, 4000000000u % 7u);
    return 0;
}
