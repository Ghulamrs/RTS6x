/* Spec: none - a failed assert: __c6xabi_abort_msg writes the message to stderr and aborts (134). */
#include <assert.h>
#include <stdio.h>

int main(void)
{
    int two = 2;
    puts("before");
    assert(two == 3);
    puts("not reached");
    return 0;
}
