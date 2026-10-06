// Spec: none - qsort on many shapes: lengths 0-40 and some longer, element sizes 1, 3, 4, 8 and 12
// (bytes and words swapped), sorted, reversed, random, all equal and few distinct inputs. Each case
// must be ordered, found by bsearch, and up to 64 equal to an insertion sort; a line counts failures.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned seed = 1;
static unsigned next() { seed = seed * 1103515245u + 12345u; return seed >> 8; }

static unsigned char data[300 * 12 + 4], copy[300 * 12 + 4];
static size_t width;

static int order(const void *a, const void *b)
{
    const unsigned char *x = (const unsigned char *)a, *y = (const unsigned char *)b;
    for (size_t k = width; k-- > 0;)
        if (x[k] != y[k]) return x[k] < y[k] ? -1 : 1;
    return 0;
}

static unsigned key(int shape, int i, int n)
{
    switch (shape) {
    case 0: return (unsigned)i;
    case 1: return (unsigned)(n - i);
    case 2: return next();
    case 3: return 7;
    default: return next() % 3;
    }
}

static int check(int n, int shape, int offset)
{
    unsigned char *a = data + offset;
    for (int i = 0; i < n; i++) {
        unsigned v = key(shape, i, n);
        for (size_t k = 0; k < width; k++) a[i * width + k] = (unsigned char)(k + 1 < width ? i * 31 + k : v);
    }
    memcpy(copy, a, n * width);
    qsort(a, n, width, order);
    for (int i = 1; i < n; i++)
        if (order(a + (i - 1) * width, a + i * width) > 0) return 1;
    for (int i = 0; i < n; i++)
        if (bsearch(copy + i * width, a, n, width, order) == 0) return 1;
    for (int i = 1; i < n && n <= 64; i++)
        for (int j = i; j > 0 && order(copy + (j - 1) * width, copy + j * width) > 0; j--)
            for (size_t k = 0; k < width; k++) {
                unsigned char t = copy[(j - 1) * width + k];
                copy[(j - 1) * width + k] = copy[j * width + k];
                copy[j * width + k] = t;
            }
    return n <= 64 && memcmp(a, copy, n * width) != 0;
}

int main()
{
    static const size_t widths[] = { 1, 3, 4, 8, 12 };
    static const int longer[] = { 63, 64, 100, 300 };
    for (unsigned w = 0; w < sizeof widths / sizeof widths[0]; w++) {
        width = widths[w];
        int bad = 0;
        for (int shape = 0; shape < 5; shape++) {
            for (int n = 0; n <= 40; n++) bad += check(n, shape, (n & 1) * (width == 1 ? 1 : 0));
            for (unsigned k = 0; k < sizeof longer / sizeof longer[0]; k++)
                if (longer[k] * width <= 300 * 12) bad += check(longer[k], shape, 0);
        }
        printf("width %u: %d\n", (unsigned)width, bad);
    }
    return 0;
}
