/* Spec: ISO C 7.11 (setlocale, localeconv in "C"), 7.14 (signal, raise, SIG_IGN, the handler reset
   to SIG_DFL), 7.20.4.5 (getenv of an unset name) and 7.19.10.4 (perror's form). */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

static volatile sig_atomic_t seen;

static void handler(int sig) { seen = sig; }

int main(void)
{
    struct lconv *lc;
    printf("setlocale %s %s %s %d\n", setlocale(LC_ALL, 0), setlocale(LC_ALL, "C"), setlocale(LC_NUMERIC, ""),
           setlocale(LC_ALL, "xx_YY") == 0);
    lc = localeconv();
    printf("lconv [%s] [%s] %d %d\n", lc->decimal_point, lc->thousands_sep, lc->frac_digits == CHAR_MAX,
           lc->n_sign_posn == CHAR_MAX);
    printf("signal %d\n", signal(SIGINT, handler) == SIG_DFL);
    printf("raise %d, seen %d\n", raise(SIGINT), (int)seen);
    printf("reset %d\n", signal(SIGINT, SIG_IGN) == SIG_DFL);
    printf("ignored %d\n", raise(SIGINT));
    printf("getenv %d\n", getenv("RTS6X_SURELY_UNSET_NAME") == 0);
    printf("strerror %d\n", strlen(strerror(0)) > 0);
    return 0;
}
