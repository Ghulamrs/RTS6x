// Spec: ISO C++11 6.7/4 (a static local is built once, the first time control passes) and
// 3.6.3/1 (destroyed after main in the reverse order of construction), through the Itanium guards
// and __cxa_atexit.
#include <stdio.h>

struct Noisy {
    int id;
    explicit Noisy(int i) : id(i) { printf("build %d\n", id); }
    ~Noisy() { printf("destroy %d\n", id); }
};

static Noisy &first() { static Noisy n(1); return n; }
static Noisy &second() { static Noisy n(2); return n; }
static int counted(int k) { static int calls = 40; return calls += k; }

int main()
{
    printf("start\n");
    second();
    first();
    for (int i = 0; i < 3; i++) printf("ids %d %d\n", first().id, second().id);
    printf("counted %d %d\n", counted(4), counted(9));
    printf("end of main\n");
    return 0;
}
