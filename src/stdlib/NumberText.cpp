// Spec: ISO C 7.20.1.4/2-8. strtoul's negated value wraps as unsigned long arithmetic does; long
// and unsigned long are 32 bits on the C6000, so 33 bits of accumulator say "out of range".

#include <errno.h>
#include <limits.h>
#include "NumberText.h"
#include "../ctype/CharacterClass.h"
#include "../misc/ErrorNumber.h"

namespace rts6x {

int NumberText::digitValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return 99;
}

unsigned long long NumberText::parse(const char *s, const char **end, int base, bool &negative)
{
    const char *p = s;
    while (CharacterClass::space((unsigned char)*p)) p++;
    negative = false;
    if (*p == '+' || *p == '-') negative = *p++ == '-';
    if ((base == 0 || base == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X') && digitValue(p[2]) < 16) {
        p += 2;
        base = 16;
    } else if (base == 0) {
        base = *p == '0' ? 8 : 10;
    }
    *end = 0;
    if (base < 2 || base > 36 || digitValue(*p) >= base) return 0;
    unsigned long long value = 0;
    for (; digitValue(*p) < base; p++) {
        value = value * (unsigned)base + (unsigned)digitValue(*p);
        if (value > 0x1FFFFFFFFull) value = 0x1FFFFFFFFull;
    }
    *end = p;
    return value;
}

long NumberText::toLong(const char *s, char **end, int base)
{
    bool negative;
    const char *stop;
    unsigned long long v = parse(s, &stop, base, negative);
    if (end) *end = (char *)(stop ? stop : s);
    if (!negative && v > (unsigned long long)LONG_MAX) { ErrorNumber::set(ERANGE); return LONG_MAX; }
    if (negative && v > (unsigned long long)LONG_MAX + 1) { ErrorNumber::set(ERANGE); return LONG_MIN; }
    return negative ? (long)(0ul - (unsigned long)v) : (long)v;
}

unsigned long NumberText::toUnsignedLong(const char *s, char **end, int base)
{
    bool negative;
    const char *stop;
    unsigned long long v = parse(s, &stop, base, negative);
    if (end) *end = (char *)(stop ? stop : s);
    if (v > (unsigned long long)ULONG_MAX) { ErrorNumber::set(ERANGE); return ULONG_MAX; }
    return negative ? 0ul - (unsigned long)v : (unsigned long)v;
}

}  // namespace rts6x
