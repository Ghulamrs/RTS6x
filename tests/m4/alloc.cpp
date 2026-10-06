// Spec: ISO C++11 18.6.1 (operator new and delete, the array and nothrow forms) and 18.6.2.3-4
// (set_new_handler answers the one before, get_new_handler the one in place).
#include <stdio.h>
#include <new>

struct Counted {
    static int live;
    int v;
    Counted() : v(7) { live++; }
    ~Counted() { live--; }
};
int Counted::live = 0;

static void handler() {}

int main()
{
    int *p = new int(41);
    Counted *one = new Counted;
    Counted *many = new Counted[5];
    printf("%d %d live %d\n", *p + 1, one->v + many[4].v, Counted::live);
    delete p;
    delete one;
    delete[] many;
    printf("live %d\n", Counted::live);
    int *q = new (std::nothrow) int[100];
    printf("nothrow %d\n", q != 0);
    delete[] q;
    std::new_handler old = std::set_new_handler(handler);
    printf("handlers %d %d %d\n", old == 0, std::get_new_handler() == handler, std::set_new_handler(0) == handler);
    char *blocks[50];
    for (int i = 0; i < 50; i++) blocks[i] = new char[1000 + i];
    for (int i = 0; i < 50; i += 2) delete[] blocks[i];
    for (int i = 1; i < 50; i += 2) delete[] blocks[i];
    delete static_cast<int *>(0);
    printf("done\n");
    return 0;
}
