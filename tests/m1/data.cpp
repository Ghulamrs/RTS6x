// Spec: none - what the startup prepares before main: .data from .cinit (runs long enough for the
// rle24 long forms), zeroed .bss and .far, a static constructor, and its destructor at exit.
#include <stdio.h>

static int small[4] = { 1, 2, 3, 4 };
static unsigned char run300[300] = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 9 };
static char text[] = "initialised text";
static int zeroes[70000];
static double fraction = 2.5;
int counter = 41;

struct Banner {
    int id;
    Banner(int i) : id(i) { printf("constructed %d\n", id); }
    ~Banner() { printf("destroyed %d\n", id); }
};
Banner first(1), second(2);

int main()
{
    int sum = 0, nonzero = 0;
    for (int i = 0; i < 4; i++) sum += small[i];
    for (int i = 0; i < 300; i++) sum += run300[i];
    for (int i = 0; i < 70000; i++) nonzero += zeroes[i] != 0;
    counter++;
    printf("sum %d, nonzero %d, text [%s], fraction %g, counter %d, ids %d %d\n",
           sum, nonzero, text, fraction, counter, first.id, second.id);
    return 3;
}
