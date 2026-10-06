// Spec: none - tools/speed: throw and catch through three frames with a destructor in each, 200 times.
#include <stdio.h>
static int built = 0, gone = 0;
struct Guard {
    Guard() { built++; }
    ~Guard() { gone++; }
};
static int deep(int n)
{
    Guard g;
    if (n == 0) throw n + 1;
    return deep(n - 1);
}
int main()
{
    int caught = 0;
    for (int i = 0; i < 200; i++) {
        try { deep(2); } catch (int e) { caught += e; }
    }
    printf("%d %d %d\n", caught, built, gone);
    return 0;
}
