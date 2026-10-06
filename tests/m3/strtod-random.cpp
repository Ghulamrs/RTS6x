// Spec: none - strtod against the host's correctly rounded strtod, bit for bit, with where it stopped
// and ERANGE: random decimals of 1 to 19 digits at every exponent, long digit strings, and exact
// halfway points between two doubles, whole, cut short or nudged up a unit in their last digit.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef ROUNDS
#define ROUNDS 1
#endif
#ifndef COUNT
#define COUNT 150
#endif
#ifndef SEED
#define SEED 0x9E3779B97F4A7C15ull
#endif

namespace {

unsigned long long state = SEED;
unsigned long long next()
{
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}
unsigned below(unsigned n) { return (unsigned)(next() >> 33) % n; }

char text[1400];
int length;
void put(char c) { text[length++] = c; }

void putNumber(int v)
{
    if (v < 0) { put('-'); v = -v; }
    char r[12];
    int n = 0;
    do { r[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n) put(r[--n]);
}

// n random digits, the first not 0, a point after `point` of them (none if point == n), an exponent.
void randomDecimal(int n, int exponentRange, int exponentLow)
{
    int point = (int)below((unsigned)n + 1);
    for (int i = 0; i < n; i++) {
        if (i == point && i > 0) put('.');
        put((char)('0' + (i == 0 ? 1 + below(9) : below(10))));
    }
    if (below(4)) { put(below(2) ? 'e' : 'E'); putNumber(exponentLow + (int)below((unsigned)exponentRange)); }
}

// A halfway point (2M + 1) 2^s between two doubles, written out in full, then perhaps cut short or nudged.
void halfway()
{
    unsigned char d[1200];
    int n = 0;
    unsigned long long m = ((next() >> 11) | (1ull << 52)) * 2 + 1;
    while (m) { d[n++] = (unsigned char)(m % 10); m /= 10; }
    // Mostly near 1, now and then anywhere from the subnormals to the top of the range.
    int shift = below(16) ? (int)below(300) - 220 : (int)below(2046) - 1075, scale = shift < 0 ? 5 : 2;
    for (int k = shift < 0 ? -shift : shift; k > 0; k--) {
        unsigned carry = 0;
        for (int i = 0; i < n; i++) { unsigned v = d[i] * (unsigned)scale + carry; d[i] = (unsigned char)(v % 10); carry = v / 10; }
        while (carry) { d[n++] = (unsigned char)(carry % 10); carry /= 10; }
    }
    // d, least significant first, is (2M + 1) 5^-s or 2^s; the value is d 10^s or d.
    int keep = n, how = (int)below(4);
    if (how == 1 || how == 2) keep = 1 + (int)below((unsigned)n);
    for (int i = n - 1; i >= n - keep; i--) {
        int v = d[i];
        if (how == 2 && i == n - keep) v++;          // nudged: may become 10, written as ':'; then cut
        put((char)('0' + (v > 9 ? 9 : v)));
    }
    int zeros = how == 3 ? (int)below(30) : 0;
    for (int z = zeros; z > 0; z--) put('0');
    put('e');
    putNumber((shift < 0 ? shift : 0) + (n - keep) - zeros);
}

}  // namespace

int main()
{
    for (int round = 0; round < ROUNDS; round++) {
        unsigned long long sum = 0;
        for (int i = 0; i < COUNT; i++) {
            length = 0;
            if (below(3) == 0) put(below(2) ? '-' : '+');
            switch (below(5)) {
            case 0: randomDecimal(1 + (int)below(19), 61, -30); break;
            case 1: randomDecimal(1 + (int)below(19), 701, -360); break;
            case 2: randomDecimal(20 + (int)below(40), 661, -350); break;
            default: halfway(); break;
            }
            if (below(4) == 0) put('x');
            text[length] = 0;
            char *end;
            errno = 0;
            double v = strtod(text, &end);
            unsigned long long bits;
            memcpy(&bits, &v, sizeof bits);
            sum = (sum ^ bits ^ ((unsigned long long)(end - text) << 1) ^ (errno == ERANGE)) * 1099511628211ull;
        }
        printf("%d %016llx\n", round, sum);
    }
    return 0;
}
