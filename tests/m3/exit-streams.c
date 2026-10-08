/* Spec: ISO C 7.20.4.3/4 - exit calls the atexit functions first, then closes every open stream and
   removes the tmpfile ones: a file and a temporary left open, written to again from an atexit function,
   and exit called from inside a function with the status 4. */
#include <stdio.h>
#include <stdlib.h>

static FILE *kept;

static void lastWords(void)
{
    printf("atexit: fputs %d, fprintf %d\n", fputs("from atexit\n", kept) >= 0, fprintf(kept, "%d\n", 4) == 2);
}

static void finish(int status)
{
    puts("exiting with the files open");
    exit(status);
}

int main(void)
{
    FILE *t = tmpfile();
    kept = fopen("rts6x-m3-exit.txt", "w");
    if (!kept || !t) { puts("no file"); return 1; }
    atexit(lastWords);
    fputs("from main\n", kept);
    fprintf(t, "temporary %d", 1);
    printf("written %d\n", (int)ftell(t));
    finish(4);
    puts("not reached");
    return 0;
}
