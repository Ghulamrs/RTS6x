// Spec: ISO C 7.20.1.4 - strtol and strtoul: white space, a sign, then digits in the base (2-36,
// or 0: decimal, 0 octal, 0x hexadecimal); the end where the number stopped, or the start if
// there was none; past the range, the nearest end of it and ERANGE.
#ifndef RTS6X_NUMBER_TEXT_H
#define RTS6X_NUMBER_TEXT_H

namespace rts6x {

class NumberText {
public:
    static long toLong(const char *s, char **end, int base);
    static unsigned long toUnsignedLong(const char *s, char **end, int base);

private:
    // The digits of s in base: their value, capped once it passes 32 bits, and whether a sign
    // said negative. end is where they stopped; a null end where there was no number.
    static unsigned long long parse(const char *s, const char **end, int base, bool &negative);
    static int digitValue(char c);
};

}  // namespace rts6x

#endif
