// Spec: none - tools/speed: qsort of 1000 ints, then bsearch of each; a checksum.
#include <stdio.h>
#include <stdlib.h>
static int order(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return x < y ? -1 : x > y;
}
int main()
{
    static int v[1000];
    unsigned seed = 12345;
    for (int i = 0; i < 1000; i++) { seed = seed * 1103515245u + 12345u; v[i] = (int)(seed >> 8); }
    qsort(v, 1000, sizeof v[0], order);
    long found = 0;
    for (int i = 0; i < 1000; i++) found += bsearch(&v[i], v, 1000, sizeof v[0], order) != 0;
    printf("%d %d %ld\n", v[0], v[999], found);
    return 0;
}
