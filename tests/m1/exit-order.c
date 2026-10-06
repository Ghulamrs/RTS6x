/* Spec: none - atexit functions run last-registered first, from exit called deep in the program,
   and one registered during exit still runs; the status is exit's argument, 9. */
#include <stdio.h>
#include <stdlib.h>

static void third(void) { puts("third registered, first run"); }
static void late(void) { puts("registered while exiting"); }
static void second(void) { puts("second"); atexit(late); }
static void first(void) { puts("first registered, last run"); }

static void deep(int n)
{
    if (n == 0) {
        puts("exiting from depth 3");
        exit(9);
    }
    deep(n - 1);
}

int main(void)
{
    atexit(first);
    atexit(second);
    atexit(third);
    deep(3);
    puts("not reached");
    return 0;
}
