// Spec: ISO C 7.19.6.1/7-8: d i signed and o u x X unsigned, at the width the length modifier
// names; the precision a minimum digit count (0 with 0 gives none); # adds 0 for o, 0x for x;
// a 0 flag is ignored when a precision is given. p is a pointer, written here as 0x and hex.
#include "Formatter.h"
#include "IntegerDigits.h"

namespace rts6x {

void Formatter::integer(const FormatSpec &spec)
{
    char c = spec.conversion();
    bool isSigned = c == 'd' || c == 'i';
    unsigned long long magnitude = 0;
    bool negative = false;
    if (isSigned) {
        long long v;
        switch (spec.length()) {
        case FormatSpec::Char: v = (signed char)va_arg(*args_, int); break;
        case FormatSpec::Short: v = (short)va_arg(*args_, int); break;
        case FormatSpec::Long: v = va_arg(*args_, long); break;
        case FormatSpec::LongLong: case FormatSpec::IntMax: v = va_arg(*args_, long long); break;
        default: v = va_arg(*args_, int); break;
        }
        negative = v < 0;
        magnitude = negative ? (unsigned long long)(-(v + 1)) + 1u : (unsigned long long)v;
    } else {
        switch (spec.length()) {
        case FormatSpec::Char: magnitude = (unsigned char)va_arg(*args_, unsigned); break;
        case FormatSpec::Short: magnitude = (unsigned short)va_arg(*args_, unsigned); break;
        case FormatSpec::Long: magnitude = va_arg(*args_, unsigned long); break;
        case FormatSpec::LongLong: case FormatSpec::IntMax: magnitude = va_arg(*args_, unsigned long long); break;
        default: magnitude = va_arg(*args_, unsigned); break;
        }
    }
    unsigned base = c == 'o' ? 8 : (c == 'x' || c == 'X') ? 16 : 10;
    unsignedField(spec, magnitude, negative, base);
}

void Formatter::unsignedField(const FormatSpec &spec, unsigned long long magnitude, bool negative, unsigned base)
{
    unsigned hi = (unsigned)(magnitude >> 32), lo = (unsigned)magnitude;
    IntegerDigits digits(hi, lo, base, spec.upper());
    size_t n = digits.length();
    size_t zeros = 0;
    if (spec.precision() > 0 && (size_t)spec.precision() > n) zeros = (size_t)spec.precision() - n;
    if (n == 0 && spec.precision() < 0) zeros = 1;                      // zero is "0"
    if (base == 8 && spec.alternate() && zeros == 0) zeros = 1;          // and #o always leads with 0

    char prefix[2];
    size_t p = 0;
    char c = spec.conversion();
    if (c == 'd' || c == 'i') p = signPrefix(spec, negative, prefix);
    else if (base == 16 && spec.alternate() && n != 0) { prefix[0] = '0'; prefix[1] = c; p = 2; }

    size_t padding = open(spec, prefix, p, zeros + n, spec.precision() < 0);
    out_.repeat('0', zeros);
    out_.put(digits.text(), n);
    close(spec, padding);
}

void Formatter::pointer(const FormatSpec &spec)
{
    void *v = va_arg(*args_, void *);
    IntegerDigits digits(0, (unsigned)(size_t)v, 16, false);
    size_t n = digits.length();
    size_t padding = open(spec, "0x", 2, n == 0 ? 1 : n, false);
    if (n == 0) out_.put('0');
    else out_.put(digits.text(), n);
    close(spec, padding);
}

}  // namespace rts6x
