// Spec: ISO C++11 5.2.7 (dynamic_cast: down, cross, ambiguous, through a virtual base, failing) and
// 5.2.8 (typeid of a polymorphic object and of a type); Itanium C++ ABI 2.9.4 for the runtime's
// type_info objects of fundamental types.
#include <stdio.h>
#include <typeinfo>

struct A { virtual ~A() {} int a; };
struct B : A { int b; };
struct C : B { int c; };
struct X { virtual ~X() {} int x; };
struct D : C, X { int d; };
struct V { virtual ~V() {} int v; };
struct L : virtual V { int l; };
struct R : virtual V { int r; };
struct J : L, R { int j; };
struct P1 : A { };
struct P2 : A { };
struct Two : P1, P2 { };

static const char *yes(const void *p) { return p ? "yes" : "no"; }

int main()
{
    D d;
    A *pa = &d;
    printf("down A->C %s, A->D %s, A->X %s\n", yes(dynamic_cast<C *>(pa)), yes(dynamic_cast<D *>(pa)),
           yes(dynamic_cast<X *>(pa)));
    X *px = &d;
    printf("cross X->B %s, same object %d\n", yes(dynamic_cast<B *>(px)), (void *)dynamic_cast<B *>(px) == (void *)static_cast<B *>(&d));
    C c;
    pa = &c;
    printf("fail A->D %s, A->X %s, A->C %s\n", yes(dynamic_cast<D *>(pa)), yes(dynamic_cast<X *>(pa)), yes(dynamic_cast<C *>(pa)));
    J j;
    V *pv = &j;
    printf("virtual V->J %s, V->L %s, V->R %s, right %d\n", yes(dynamic_cast<J *>(pv)), yes(dynamic_cast<L *>(pv)),
           yes(dynamic_cast<R *>(pv)), dynamic_cast<R *>(pv) == static_cast<R *>(&j));
    Two t;
    P1 *p1 = &t;
    A *viaP1 = p1;
    printf("ambiguous A->Two %s, P1->P2 %s, A->P2 %s\n", yes(dynamic_cast<Two *>(viaP1)), yes(dynamic_cast<P2 *>(p1)),
           yes(dynamic_cast<P2 *>(viaP1)));
    printf("void* top %d\n", dynamic_cast<void *>(px) == (void *)&d);
    A *pc = &c;
    printf("typeid %d %d %d\n", typeid(*pc) == typeid(C), typeid(*pc) == typeid(A), typeid(int) == typeid(unsigned));
    printf("names %s %s %s %s\n", typeid(int).name(), typeid(const char *).name(), typeid(double *).name(),
           typeid(c).name());
    return 0;
}
