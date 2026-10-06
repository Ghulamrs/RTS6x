// Spec: ISO C++11 15.2-15.3 - the unwinder's lookup and frame restore under load: mutual recursion
// across functions far apart in the index, a throw caught and finished inside a destructor run while
// another unwinds, frames holding many callee-saved values, and handlers found again and again.
#include <stdio.h>

static int built = 0, gone = 0;
struct Mark {
    int id;
    explicit Mark(int i) : id(i) { built++; }
    ~Mark() { gone++; }
};

static int ping(int n, int k);
static int pong(int n, int k)
{
    Mark m(n);
    if (n == 0) throw k;
    return ping(n - 1, k) + m.id;
}
static int ping(int n, int k)
{
    Mark m(-n);
    if (n == 0) throw k + 1000;
    return pong(n - 1, k) - m.id;
}

static int inner(int v)
{
    try {
        if (v & 1) throw v * 3;
        return v;
    } catch (int e) {
        return e;
    }
}

// Its destructor throws and catches inside itself while the outer exception is being unwound.
struct Juggler {
    int *sink;
    explicit Juggler(int *s) : sink(s) {}
    ~Juggler() { *sink += inner(7) + inner(4); }
};

static int juggle(int n, int *sink)
{
    Juggler j(sink);
    if (n == 0) throw 5;
    return juggle(n - 1, sink) + 1;
}

// Values kept across the call in callee-saved registers, which the handler's frame must get back.
static int crowd(int depth, int seed)
{
    int a = seed + 1, b = seed * 7, c = seed ^ 0x55, d = seed - 9, e = seed * seed, f = seed + 100;
    int g = seed * 3 + 2, h = seed << 3, i = seed | 0x100, j = seed & 0xff;
    int got = 0;
    try {
        if (depth > 0) got = crowd(depth - 1, seed + 1);
        else throw seed;
    } catch (int x) {
        if (depth < 3) throw;
        got = x;
    }
    return got + a + b + c + d + e + f + g + h + i + j;
}

int main()
{
    long total = 0;
    for (int round = 0; round < 6; round++) {
        try {
            total += ping(round + 3, round);
        } catch (int e) {
            total += e;
        }
    }
    printf("ping-pong %ld built %d gone %d\n", total, built, gone);
    int sink = 0;
    for (int round = 0; round < 3; round++) {
        try {
            juggle(round + 2, &sink);
        } catch (int e) {
            sink += e * 100;
        }
    }
    printf("juggle %d\n", sink);
    printf("crowd %d %d\n", crowd(3, 3), crowd(5, 11));
    return 0;
}
