/* Spec: none - the layouts of src/internal/rts6x.h, held to a compiler's own headers. Run on the
   VM6747 emulator, which carries its own run-time, it prints each disagreement and exits with
   their count; offsets are measured on an object, since offsetof does not fold to a constant. */
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <locale.h>
#include "rts6x.h"

static int bad;
static void check(const char *what, long got, long want)
{
    if (got != want) { printf("layout: %s is %ld, rts6x.h says %ld\n", what, got, want); bad++; }
}

int main(void)
{
    FILE f;
    char *b = (char *)&f;
    check("sizeof(FILE)", (long)sizeof(FILE), RTS6X_FILE_SIZE);
    check("FILE.fd", (long)((char *)&f.fd - b), RTS6X_FILE_FD);
    check("FILE.buf", (long)((char *)&f.buf - b), RTS6X_FILE_BUF);
    check("FILE.pos", (long)((char *)&f.pos - b), RTS6X_FILE_POS);
    check("FILE.bufend", (long)((char *)&f.bufend - b), RTS6X_FILE_BUFEND);
    check("FILE.buff_stop", (long)((char *)&f.buff_stop - b), RTS6X_FILE_BUFF_STOP);
    check("FILE.flags", (long)((char *)&f.flags - b), RTS6X_FILE_FLAGS);
    check("sizeof(_ftable)", (long)sizeof(_ftable), (long)RTS6X_FTABLE_COUNT * RTS6X_FILE_SIZE);
    check("sizeof(jmp_buf)", (long)sizeof(jmp_buf), RTS6X_JMP_BUF_SIZE);
    check("sizeof(struct tm)", (long)sizeof(struct tm), RTS6X_TM_SIZE);
    check("sizeof(struct lconv)", (long)sizeof(struct lconv), RTS6X_LCONV_SIZE);
    check("sizeof(time_t)", (long)sizeof(time_t), RTS6X_TIME_T_SIZE);
    check("sizeof(clock_t)", (long)sizeof(clock_t), RTS6X_CLOCK_T_SIZE);
    return bad;
}
