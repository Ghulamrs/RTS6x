// Spec: ISO C 7.21.2.1, 7.21.2.2, 7.21.6.1, 7.21.4.1, 7.21.5.1 - memcpy, memmove, memset, memcmp,
// memchr. memmove copies backwards when the destination starts inside the source.

#include "Memory.h"

namespace rts6x {

void *Memory::copy(void *to, const void *from, size_t n)
{
    unsigned char *d = (unsigned char *)to;
    const unsigned char *s = (const unsigned char *)from;
    if (aligned(d, s))
        for (; n >= 4; n -= 4, d += 4, s += 4) *(unsigned *)d = *(const unsigned *)s;
    while (n--) *d++ = *s++;
    return to;
}

void *Memory::move(void *to, const void *from, size_t n)
{
    unsigned char *d = (unsigned char *)to;
    const unsigned char *s = (const unsigned char *)from;
    if (d <= s || d >= s + n) return copy(to, from, n);
    while (n--) d[n] = s[n];
    return to;
}

void *Memory::fill(void *to, int value, size_t n)
{
    unsigned char *d = (unsigned char *)to;
    unsigned char b = (unsigned char)value;
    if (((size_t)d & 3) == 0) {
        unsigned word = b * 0x01010101u;
        for (; n >= 4; n -= 4, d += 4) *(unsigned *)d = word;
    }
    while (n--) *d++ = b;
    return to;
}

int Memory::compare(const void *a, const void *b, size_t n)
{
    const unsigned char *x = (const unsigned char *)a, *y = (const unsigned char *)b;
    for (; n; n--, x++, y++)
        if (*x != *y) return *x < *y ? -1 : 1;
    return 0;
}

void *Memory::find(const void *in, int value, size_t n)
{
    const unsigned char *p = (const unsigned char *)in;
    unsigned char b = (unsigned char)value;
    for (; n; n--, p++)
        if (*p == b) return (void *)p;
    return 0;
}

}  // namespace rts6x
