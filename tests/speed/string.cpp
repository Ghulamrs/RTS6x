// Spec: none - tools/speed: memcpy, memset, strlen, strcmp and strchr on a 1 KB buffer, 300 times.
#include <stdio.h>
#include <string.h>
int main()
{
    static char a[1024], b[1024];
    unsigned sum = 0;
    for (int i = 0; i < 300; i++) {
        memset(a, 'a' + i % 26, sizeof a - 1);
        a[sizeof a - 1] = 0;
        memcpy(b, a, sizeof a);
        b[i % 1000] = 'Z';
        sum += (unsigned)strlen(b) + (unsigned)(strcmp(a, b) > 0) + (unsigned)(strchr(b, 'Z') - b);
    }
    printf("%u\n", sum);
    return 0;
}
