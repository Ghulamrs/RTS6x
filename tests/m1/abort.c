/* Spec: none - abort ends the program without calling atexit's functions: status 134. */
#include <stdio.h>
#include <stdlib.h>

static void never(void) { puts("an atexit function ran after abort"); }

int main(void)
{
    atexit(never);
    puts("aborting");
    abort();
    return 0;
}
