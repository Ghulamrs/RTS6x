// Spec: none - the word and doubleword paths of <string.h> against a byte-at-a-time reference: every
// source and destination alignment 0-7, lengths 0-72 and some longer, memmove overlapping both ways,
// strings ending at every byte of a doubleword. Each line is a function and how many cases failed.
#include <stdio.h>
#include <string.h>

static unsigned char src[640], dst[640], ref[640], big[640];

static void pattern(unsigned char *p, int n, int seed)
{
    for (int i = 0; i < n; i++) p[i] = (unsigned char)(i * 7 + seed * 13 + 1);
}

static int sameAs(const unsigned char *a, const unsigned char *b, int n)
{
    for (int i = 0; i < n; i++) if (a[i] != b[i]) return 0;
    return 1;
}

static int lengths[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 23, 24, 25, 31,
                         32, 33, 39, 40, 41, 47, 48, 49, 55, 56, 57, 63, 64, 65, 71, 72, 100, 255, 256, 300 };
static const int count = sizeof lengths / sizeof lengths[0];

static int checkCopy()
{
    int bad = 0;
    for (int so = 0; so < 8; so++)
        for (int d = 0; d < 8; d++)
            for (int k = 0; k < count; k++) {
                int n = lengths[k];
                pattern(src, 320, so + n); pattern(dst, 320, 99); pattern(ref, 320, 99);
                for (int i = 0; i < n; i++) ref[8 + d + i] = src[so + i];
                if (memcpy(dst + 8 + d, src + so, n) != dst + 8 + d || !sameAs(dst, ref, 320)) bad++;
            }
    return bad;
}

static int shifts[] = { -40, -33, -32, -31, -25, -24, -17, -16, -9, -8, -7, -5, -3, -2, -1, 0,
                        1, 2, 3, 5, 7, 8, 9, 16, 17, 24, 25, 31, 32, 33, 40 };

static int checkMove()
{
    int bad = 0;
    for (unsigned s = 0; s < sizeof shifts / sizeof shifts[0]; s++)
        for (int k = 0; k < count; k++)
            for (int a = 0; a < 8; a += 3) {
                int n = lengths[k], from = 200 + a, to = from + shifts[s], low = 150, high = from + n + 50;
                pattern(big + low, high - low, n + s);
                memcpy(ref + low, big + low, high - low);
                for (int i = 0; i < n; i++) ref[to + i] = big[from + i];
                if (memmove(big + to, big + from, n) != big + to || !sameAs(big + low, ref + low, high - low)) bad++;
            }
    return bad;
}

static int checkFill()
{
    int bad = 0;
    for (int d = 0; d < 8; d++)
        for (int k = 0; k < count; k++) {
            int n = lengths[k];
            pattern(dst, 320, d); pattern(ref, 320, d);
            for (int i = 0; i < n; i++) ref[8 + d + i] = (unsigned char)(0x5A + n);
            if (memset(dst + 8 + d, 0x15A + n, n) != dst + 8 + d || !sameAs(dst, ref, 320)) bad++;
        }
    return bad;
}

static int sign(int v) { return v < 0 ? -1 : v > 0; }

static int checkCompare()
{
    int bad = 0;
    for (int so = 0; so < 8; so++)
        for (int d = 0; d < 8; d++)
            for (int k = 0; k < count; k++) {
                int n = lengths[k];
                pattern(src + so, n, 3); pattern(dst + d, n, 3);
                if (memcmp(src + so, dst + d, n) != 0) bad++;
                for (int at = 0; at < n; at += (n > 16 ? 9 : 1)) {
                    dst[d + at] = (unsigned char)(src[so + at] + 0x80);
                    int want = src[so + at] < dst[d + at] ? -1 : 1;
                    if (sign(memcmp(src + so, dst + d, n)) != want) bad++;
                    if (sign(memcmp(dst + d, src + so, n)) != -want) bad++;
                    dst[d + at] = src[so + at];
                }
            }
    return bad;
}

static void text(char *p, int n, int seed)
{
    for (int i = 0; i < n; i++) p[i] = (char)('a' + (i + seed) % 26);
    p[n] = 0;
}

static int checkStrings()
{
    int bad = 0;
    char *a = (char *)src, *b = (char *)dst, *c = (char *)ref;
    for (int so = 0; so < 8; so++)
        for (int n = 0; n < 72; n += (n < 24 ? 1 : 5)) {
            text(a + so, n, so);
            if (strlen(a + so) != (size_t)n) bad++;
            if (strchr(a + so, 0) != a + so + n) bad++;
            if (n > 0 && strchr(a + so, a[so + n - 1]) != a + so + (n - 1 < 26 ? n - 1 : (n - 1) % 26)) bad++;
            if (strchr(a + so, 'A') != 0) bad++;
            if (n > 0 && strrchr(a + so, a[so]) != a + so + (n - 1) / 26 * 26) bad++;
            for (int d = 0; d < 8; d++) {
                memset(b, 'Q', n + 20);
                if (strcpy(b + d, a + so) != b + d || strcmp(b + d, a + so) != 0 || b[d + n + 1] != 'Q') bad++;
                if (strncmp(b + d, a + so, n + 3) != 0) bad++;
                for (int at = 0; at < n; at += (n > 8 ? 5 : 1)) {
                    b[d + at] = (char)(b[d + at] + 1);
                    if (strcmp(a + so, b + d) >= 0 || strcmp(b + d, a + so) <= 0) bad++;
                    if (strncmp(a + so, b + d, at) != 0 || strncmp(a + so, b + d, at + 1) >= 0) bad++;
                    b[d + at] = (char)0xE0;
                    if (strcmp(a + so, b + d) >= 0) bad++;
                    b[d + at] = 0;
                    if (strcmp(a + so, b + d) <= 0 || strcmp(b + d, a + so) >= 0) bad++;
                    b[d + at] = a[so + at];
                }
                memset(c, 'R', n + 20);
                text(c + d, 5, 9);
                if (strcat(c + d, a + so) != c + d || strlen(c + d) != (size_t)n + 5 || c[d + n + 6] != 'R') bad++;
                memset(c, 'R', n + 20);
                if (strncpy(c + d, a + so, n + 4) != c + d || strcmp(c + d, a + so) != 0 ||
                    c[d + n + 3] != 0 || c[d + n + 4] != 'R') bad++;
            }
        }
    return bad;
}

int main()
{
    printf("memcpy %d\n", checkCopy());
    printf("memmove %d\n", checkMove());
    printf("memset %d\n", checkFill());
    printf("memcmp %d\n", checkCompare());
    printf("strings %d\n", checkStrings());
    return 0;
}
