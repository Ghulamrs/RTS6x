/* Spec: ISO C 7.13 - setjmp answers 0, then the value longjmp passes (1 for 0); a longjmp out of
   deep recursion lands with the saved frame intact. */
#include <stdio.h>
#include <setjmp.h>

static jmp_buf outer, inner;
static int depth;

static int dive(int n)
{
    volatile int local[8];
    local[0] = n;
    if (n == 0) longjmp(inner, 77);
    return dive(n - 1) + local[0];
}

static void zero(void) { longjmp(outer, 0); }

int main(void)
{
    volatile int kept = 5;
    int r;
    r = setjmp(outer);
    if (r == 0) {
        printf("first pass\n");
        kept = 6;
        zero();
    }
    printf("back with %d, kept %d\n", r, kept);
    r = setjmp(inner);
    if (r == 0) {
        depth = dive(50);
        printf("not reached\n");
    }
    printf("from depth with %d\n", r);
    if (setjmp(inner) == 0) longjmp(inner, -3);
    else printf("third ok\n");
    return 0;
}
