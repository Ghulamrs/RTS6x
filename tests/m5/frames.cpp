// Spec: ISO C++11 15.2-15.3 and SPRAB89B 11.5 - the unwinder's frame contract (ANALYSIS 4): every
// function between a throw and its catch saved A15 in the caller's word and B3 below it, whatever else
// its frame holds. Each shape of frame cpp11 emits stands between the throw and the handler in turn.
#include <stdio.h>
#include <stdarg.h>

static int gone = 0;
struct Mark { int id; explicit Mark(int i) : id(i) {} ~Mark() { gone += id; } };
struct Pair { int a, b; };

static void thrower(int what)
{
    if (what == 1) throw 1;
    if (what == 2) throw 2.5;
    throw Pair();
}

// No locals, one call: A15 and B3 alone in the frame.
static void plain(int what) { thrower(what); }

// Values live across the call, so the callee-saved registers are saved and come back.
static int heavy(int what, int seed)
{
    int a = seed * 3, b = seed ^ 7, c = seed + 11, d = seed * seed, e = seed - 5, f = seed << 2, g = seed / 3, h = seed % 7;
    if (seed > 100) thrower(what);
    return a + b + c + d + e + f + g + h;
}

// A frame past 124 bytes, so its slots are addressed with an offset register.
static int wide(int what, int n)
{
    int slots[80];
    for (int i = 0; i < 80; i++) slots[i] = i * n;
    if (n > 0) thrower(what);
    int sum = 0;
    for (int i = 0; i < 80; i++) sum += slots[i];
    return sum;
}

// A variadic frame, its arguments read before the throw.
static int spread(int what, int count, ...)
{
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) sum += va_arg(args, int);
    va_end(args);
    if (sum > 0) thrower(what);
    return sum;
}

// A struct returned through the hidden pointer, the throw before it is filled.
static Pair twice(int what, int v)
{
    Pair p;
    if (v > 0) thrower(what);
    p.a = v; p.b = v * 2;
    return p;
}

// Doubles live across the call: the saved pairs.
static double scaled(int what, double x)
{
    double y = x * 1.5, z = x - 0.25;
    if (x > 0) thrower(what);
    return y + z;
}

// Recursion with a destructor at every level, and the throw through a function pointer.
static int descend(int what, int n, void (*go)(int))
{
    Mark m(n);
    if (n == 0) go(what);
    return descend(what, n - 1, go) + 1;
}

static int count(int what, int shape)
{
    try {
        switch (shape) {
        case 0: plain(what); break;
        case 1: return heavy(what, 200);
        case 2: return wide(what, 3);
        case 3: return spread(what, 3, 4, 5, 6);
        case 4: return twice(what, 8).a;
        case 5: return (int)scaled(what, 2.0);
        default: return descend(what, 3, thrower);
        }
    } catch (int e) {
        return 10 + e;
    } catch (double d) {
        return 20 + (int)d;
    } catch (Pair &) {
        return 30;
    }
    return -1;
}

int main()
{
    int total = 0;
    for (int shape = 0; shape < 7; shape++)
        for (int what = 1; what <= 3; what++) total += count(what, shape);
    printf("caught %d, marks gone %d\n", total, gone);
    printf("no throw: %d %d %d %d %d\n", heavy(0, 5), wide(0, 0), spread(0, 2, -3, 1), twice(0, 0).b, (int)scaled(0, -2.0));
    return 0;
}
