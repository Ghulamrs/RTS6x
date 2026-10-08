/* Spec: ISO C 7.20.4.1 - abort raises SIGABRT, so a handler installed for it runs; when the handler
   returns the program still ends abnormally (status 134), and no atexit function is called. The handler
   writes to stderr: a host's stdout is flushed by abort before the signal, and nothing after. */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static void never(void) { puts("an atexit function ran after abort"); }
static void handler(int sig) { fprintf(stderr, "SIGABRT handler %d\n", sig == SIGABRT); }

int main(void)
{
    atexit(never);
    printf("installed %d\n", signal(SIGABRT, handler) == SIG_DFL);
    puts("aborting");
    abort();
    return 0;
}
