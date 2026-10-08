/* Spec: ISO C 7.19.6.1/14, 7.19.7.3-4 (output to a stream not open for writing: a negative value or
   EOF, the error indicator set), 7.19.5.3 (an empty mode is no mode) and 7.19.9.2 (a seek the host
   refuses leaves the position where it was). The review of 2026-10-08, F1, F10 and F11. */
#include <stdio.h>
#include <string.h>
int main(void)
{
    FILE *f;
    int c;
    printf("fprintf to stdin %d\n", fprintf(stdin, "x%d", 1) < 0);
    printf("fputs %d, fputc %d, ferror %d\n", fputs("abc", stdin) == EOF, fputc('a', stdin) == EOF, ferror(stdin) != 0);
    clearerr(stdin);
    printf("cleared %d\n", ferror(stdin));
    printf("empty mode %d\n", fopen("rts6x-m3-r.txt", "") == 0);
    f = fopen("rts6x-m3-r.txt", "w");
    fputs("abcdef", f);
    fclose(f);
    f = fopen("rts6x-m3-r.txt", "r");
    c = fgetc(f); c = fgetc(f);
    printf("read %c, ", c);
    printf("bad seek %d, ", fseek(f, -5L, SEEK_SET));
    printf("then %c, ftell %ld\n", fgetc(f), ftell(f));
    printf("bad whence %d, then %c\n", fseek(f, 0L, 7), fgetc(f));
    fclose(f);
    remove("rts6x-m3-r.txt");
    return 0;
}
