// Spec: none - the ten division helpers called by name, held to a reference that divides a bit at a
// time with shifts, compares and subtractions alone: every edge pair (0, 1, -1, the extremes, powers
// of two and their neighbours) and pseudo-random pairs of every width. Prints the count that differ.
#include <stdio.h>

extern "C" {
int __c6xabi_divi(int, int);
unsigned __c6xabi_divu(unsigned, unsigned);
int __c6xabi_remi(int, int);
unsigned __c6xabi_remu(unsigned, unsigned);
// The divrem pair returns the quotient in A4 and the remainder in A5: a 64-bit value's two halves.
unsigned long long __c6xabi_divremi(int, int);
unsigned long long __c6xabi_divremu(unsigned, unsigned);
long long __c6xabi_divlli(long long, long long);
unsigned long long __c6xabi_divull(unsigned long long, unsigned long long);
long long __c6xabi_remlli(long long, long long);
unsigned long long __c6xabi_remull(unsigned long long, unsigned long long);
}

namespace {

// Shift-and-subtract division: d is shifted up under n, then one quotient bit a turn comes down, so a
// pair costs the quotient's bits and not the dividend's. n / d in the return, n % d in r; d is never 0.
unsigned slowDivide(unsigned n, unsigned d, unsigned &r)
{
    unsigned q = 0, m = d;
    int s = 0;
    while ((m >> 31) == 0 && (m << 1) <= n) { m <<= 1; s++; }
    for (; s >= 0; s--, m >>= 1) {
        q <<= 1;
        if (n >= m) { n -= m; q |= 1u; }
    }
    r = n;
    return q;
}

unsigned long long slowDivide(unsigned long long n, unsigned long long d, unsigned long long &r)
{
    unsigned long long q = 0, m = d;
    int s = 0;
    while ((m >> 63) == 0 && (m << 1) <= n) { m <<= 1; s++; }
    for (; s >= 0; s--, m >>= 1) {
        q <<= 1;
        if (n >= m) { n -= m; q |= 1u; }
    }
    r = n;
    return q;
}

class Checker {
public:
    Checker() : wrong_(0), checked_(0) {}

    void narrow(unsigned x, unsigned y)
    {
        if (y == 0) return;
        unsigned ur, uq = slowDivide(x, y, ur);
        unsigned long long both = __c6xabi_divremu(x, y);
        expect("divu", x, y, __c6xabi_divu(x, y), uq);
        expect("remu", x, y, __c6xabi_remu(x, y), ur);
        expect("divremu", x, y, (unsigned)both, uq);
        expect("divremu %", x, y, (unsigned)(both >> 32), ur);
        int sx = (int)x, sy = (int)y;
        if (sx == -2147483647 - 1 && sy == -1) return;
        bool nx = sx < 0, ny = sy < 0;
        unsigned sr, sq = slowDivide(nx ? 0u - x : x, ny ? 0u - y : y, sr);
        if (nx != ny) sq = 0u - sq;
        if (nx) sr = 0u - sr;
        unsigned long long sboth = __c6xabi_divremi(sx, sy);
        expect("divi", x, y, (unsigned)__c6xabi_divi(sx, sy), sq);
        expect("remi", x, y, (unsigned)__c6xabi_remi(sx, sy), sr);
        expect("divremi", x, y, (unsigned)sboth, sq);
        expect("divremi %", x, y, (unsigned)(sboth >> 32), sr);
    }

    void wide(unsigned long long x, unsigned long long y)
    {
        if (y == 0) return;
        unsigned long long ur, uq = slowDivide(x, y, ur);
        expectWide("divull", x, y, __c6xabi_divull(x, y), uq);
        expectWide("remull", x, y, __c6xabi_remull(x, y), ur);
        long long sx = (long long)x, sy = (long long)y;
        if (x == 0x8000000000000000ull && sy == -1) return;
        bool nx = sx < 0, ny = sy < 0;
        unsigned long long sr, sq = slowDivide(nx ? 0ull - x : x, ny ? 0ull - y : y, sr);
        if (nx != ny) sq = 0ull - sq;
        if (nx) sr = 0ull - sr;
        expectWide("divlli", x, y, (unsigned long long)__c6xabi_divlli(sx, sy), sq);
        expectWide("remlli", x, y, (unsigned long long)__c6xabi_remlli(sx, sy), sr);
    }

    int wrong() const { return wrong_; }
    int checked() const { return checked_; }

private:
    void expect(const char *what, unsigned x, unsigned y, unsigned got, unsigned want)
    {
        checked_++;
        if (got == want) return;
        if (wrong_++ < 12) printf("%s(%08x, %08x): %08x, wanted %08x\n", what, x, y, got, want);
    }

    void expectWide(const char *what, unsigned long long x, unsigned long long y, unsigned long long got,
                    unsigned long long want)
    {
        checked_++;
        if (got == want) return;
        if (wrong_++ < 12) printf("%s(%016llx, %016llx): %016llx, wanted %016llx\n", what, x, y, got, want);
    }

    int wrong_;
    int checked_;
};

unsigned long long state = 0x9E3779B97F4A7C15ull;
unsigned long long next()
{
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

}  // namespace

int main()
{
    Checker c;
    // Edges: each power of two, one either side of it, and their negations - in both widths.
    unsigned narrow[100];
    int nn = 0;
    for (int i = 0; i < 32; i++) {
        unsigned p = 1u << i;
        narrow[nn++] = p;
        narrow[nn++] = p - 1;
        narrow[nn++] = p + 1;
    }
    narrow[nn++] = 0xFFFFFFFFu;
    narrow[nn++] = 0xFFFFFFFEu;
    narrow[nn++] = 7u;
    narrow[nn++] = 0u - 7u;
    for (int i = 0; i < nn; i++)
        for (int j = 0; j < nn; j++) c.narrow(narrow[i], narrow[j]);
    unsigned long long wideEdge[200];
    int nw = 0;
    for (int i = 0; i < 64; i++) {
        unsigned long long p = 1ull << i;
        wideEdge[nw++] = p;
        wideEdge[nw++] = p - 1;
        wideEdge[nw++] = p + 1;
    }
    wideEdge[nw++] = 0xFFFFFFFFFFFFFFFFull;
    wideEdge[nw++] = 0xFFFFFFFFFFFFFFFEull;
    wideEdge[nw++] = 1000003ull;
    wideEdge[nw++] = 0ull - 1000003ull;
    for (int i = 0; i < nw; i += 9)
        for (int j = 0; j < nw; j++) c.wide(wideEdge[i], wideEdge[j]);
    // Random pairs, each operand cut to a random width, and quotients near their maximum.
    for (int i = 0; i < 800; i++) {
        unsigned long long a = next(), b = next(), s = next();
        c.narrow((unsigned)a >> (s & 31), (unsigned)b >> ((s >> 8) & 31));
        c.wide(a >> ((s >> 16) & 63), b >> ((s >> 24) & 63));
        unsigned long long d = b >> ((s >> 32) & 63);
        if (d != 0) {
            unsigned long long r;
            unsigned long long q = slowDivide(0xFFFFFFFFFFFFFFFFull, d, r);
            c.wide(q * d + (a % 2 ? d - 1 : 0), d);
            unsigned nd = (unsigned)d, nr;
            if (nd != 0) c.narrow(slowDivide(0xFFFFFFFFu, nd, nr) * nd + (a % 2 ? nd - 1 : 0), nd);
        }
    }
    printf("%d of %d division results differ from the reference\n", c.wrong(), c.checked());
    return c.wrong() != 0;
}
