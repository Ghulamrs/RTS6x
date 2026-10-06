// Spec: ISO C++11 15.1/4 and 15.3 - a throw with the heap all but spent: the exception's own room
// found, nothing left beside it for the unwinder's sorted copy of the index, which then reads the
// index where it lies - the same handlers found, the same destructors run.
#include <stdio.h>
#include <stdlib.h>

static int gone = 0;
struct Mark { ~Mark() { gone++; } };

static int down(int n)
{
    Mark m;
    if (n == 0) throw 9;
    return down(n - 1) + 1;
}

int main()
{
    // Blocks of 256 bytes, then of 16, until none is had; one of 256 back, room for the exception alone.
    void **big = 0, **small = 0;
    for (void *p; (p = malloc(256)) != 0;) { *(void **)p = big; big = (void **)p; }
    for (void *p; (p = malloc(16)) != 0;) { *(void **)p = small; small = (void **)p; }
    void **spare = big;
    big = (void **)*big;
    free(spare);
    int caught = 0;
    for (int i = 0; i < 3; i++) {
        try { down(4); } catch (int e) { caught += e; }
    }
    while (small) { void **next = (void **)*small; free(small); small = next; }
    while (big) { void **next = (void **)*big; free(big); big = next; }
    try { down(2); } catch (int e) { caught += e; }
    printf("caught %d gone %d\n", caught, gone);
    return 0;
}
