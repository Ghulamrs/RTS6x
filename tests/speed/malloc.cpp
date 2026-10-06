// Spec: none - tools/speed: malloc and free of mixed sizes, 3000 times with 32 live; the bytes written.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
    char *live[32] = { 0 };
    unsigned long total = 0;
    for (int i = 0; i < 3000; i++) {
        int slot = (i * 7) % 32;
        free(live[slot]);
        unsigned size = 8 + (unsigned)(i * 37) % 500;
        live[slot] = (char *)malloc(size);
        memset(live[slot], i & 0xff, size);
        total += size;
    }
    for (int k = 0; k < 32; k++) free(live[k]);
    printf("%lu\n", total);
    return 0;
}
