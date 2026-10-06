// Spec: ISO C++11 15 - throw and catch: by value, by a base's reference, by a pointer to a base,
// catch (...), rethrow, a try inside a handler, objects destroyed in reverse as the stack unwinds,
// and locals that live in callee-saved registers coming back intact in the handler's frame.
#include <stdio.h>
#include <new>
#include <exception>

struct Base { int v; explicit Base(int x) : v(x) {} virtual ~Base() {} };
struct Other { int o; Other() : o(5) {} virtual ~Other() {} };
struct Derived : Other, Base { explicit Derived(int x) : Base(x) {} };
struct Noisy { int id; explicit Noisy(int i) : id(i) { printf("+%d ", id); } ~Noisy() { printf("-%d ", id); } };

static int depth(int n, int what)
{
    Noisy here(n);
    if (n == 0) {
        if (what == 0) throw 42;
        if (what == 1) throw Derived(7);
        if (what == 2) throw static_cast<Base *>(new Derived(9));
        throw 2.5;
    }
    return depth(n - 1, what) + 1;
}

static int busy(int seed)
{
    int a = seed, b = seed * 3, c = seed + 11, d = seed ^ 5, total = 0;
    for (int i = 0; i < 4; i++) {
        try {
            depth(2, i);
        } catch (int e) {
            total += e + a + b;
        } catch (Base &e) {
            total += e.v + c;
        } catch (Base *p) {
            total += p->v + d;
            delete p;
        } catch (...) {
            total += 1000;
        }
        printf("| %d\n", total);
    }
    return total + a + b + c + d;
}

static void rethrower()
{
    try {
        throw Derived(3);
    } catch (Base &b) {
        printf("first %d\n", b.v);
        b.v = 30;
        throw;
    }
}

int main()
{
    printf("busy %d\n", busy(4));
    try {
        rethrower();
    } catch (Other &o) {
        printf("other %d\n", o.o);
    }
    try {
        rethrower();
    } catch (Base &b) {
        printf("again %d\n", b.v);
        try {
            throw 'x';
        } catch (char c) {
            printf("inner %c, uncaught %d\n", c, (int)std::uncaught_exception());
        }
    }
    try {
        size_t huge = (size_t)-1 / 2;
        char *p = new char[huge];
        printf("allocated %p\n", (void *)p);
    } catch (std::bad_alloc &) {
        printf("bad_alloc\n");
    }
    printf("done %d\n", (int)std::uncaught_exception());
    return 0;
}
