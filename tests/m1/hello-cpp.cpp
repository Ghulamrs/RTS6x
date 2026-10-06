/* Spec: none - the M1 program: puts, putchar, fputs and printf through RTS6x alone, built by cpp11. */
#include <stdio.h>

int main()
{
    puts("hello, C6747");
    fputs("from RTS6x ", stdout);
    putchar('!');
    putchar('\n');
    printf("%d + %d = %d\n", 2, 3, 2 + 3);
    return 0;
}
