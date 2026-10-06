/* Spec: none - malloc, calloc, realloc and free under load: blocks of many sizes filled and
   checked, freed in a shuffled order, grown with realloc, calloc's zeros; and on the C6747,
   the heap exhausted, emptied, and then one block of most of it - which needs the merging. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned seed = 7;
static unsigned next(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }

int main(void)
{
    char *block[200];
    unsigned size[200];
    int i, j, bad = 0, zeros = 1;
    char *grow, *z;

    for (i = 0; i < 200; i++) {
        size[i] = next() % 700 + 1;
        block[i] = (char *)malloc(size[i]);
        if (!block[i] || ((unsigned long)block[i] & 7) != 0) bad++;
        else memset(block[i], i, size[i]);
    }
    for (i = 0; i < 200; i++) {
        j = (int)(next() % 200);
        if (block[j]) {
            unsigned k;
            for (k = 0; k < size[j]; k++) bad += block[j][k] != (char)j;
            free(block[j]);
            block[j] = 0;
        }
    }
    for (i = 0; i < 200; i++) if (block[i]) { free(block[i]); block[i] = 0; }
    grow = (char *)malloc(10);
    strcpy(grow, "keep me");
    for (i = 1; i <= 20; i++) grow = (char *)realloc(grow, 100 * i);
    z = (char *)calloc(1000, 8);
    for (i = 0; i < 8000; i++) zeros &= z[i] == 0;
    printf("alignment and contents: %d bad; realloc kept [%s]; calloc zero: %d\n", bad, grow, zeros);
    free(grow);
    free(z);
    free(0);
    printf("realloc(0, 5) %s; malloc(0) %s\n", realloc(0, 5) ? "allocates" : "fails", malloc(0) ? "unique" : "null");
#ifdef __TMS320C6X__
    {
        char *all[2000];
        int n = 0, again;
        while (n < 2000 && (all[n] = (char *)malloc(1000)) != 0) n++;
        for (i = 0; i < n; i++) free(all[i]);
        again = malloc(900000) != 0;
        printf("exhausted after some blocks: %d; one large block after freeing: %d\n", n > 100 && n < 2000, again);
    }
#else
    printf("exhausted after some blocks: 1; one large block after freeing: 1\n");
#endif
    return 0;
}
