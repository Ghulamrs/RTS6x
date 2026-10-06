// Spec: ISO C 7.19.5.3 and POSIX fileno - two files open at once, one closed and its descriptor
// taken again by a third, each written and read back through its own: the host files every request
// under the descriptor the target chose at open, so a mix-up shows as one file's text in another.

#include <stdio.h>

static void show(const char *name)
{
    char line[32];
    FILE *f = fopen(name, "r");
    if (!f) { printf("%s: not there\n", name); return; }
    if (fgets(line, sizeof line, f)) printf("%s: %s", name, line);
    fclose(f);
    remove(name);
}

int main(void)
{
    FILE *a = fopen("rts6x-m6-a.txt", "w");
    FILE *b = fopen("rts6x-m6-b.txt", "wb");
    FILE *c;
    if (!a || !b) { printf("no file\n"); return 1; }
    printf("a %d b %d\n", fileno(a), fileno(b));
    fputs("first\n", a);
    fputs("second\n", b);
    fclose(a);
    c = fopen("rts6x-m6-c.txt", "w");
    if (!c) { printf("no third\n"); return 2; }
    printf("c %d\n", fileno(c));
    fputs("third\n", c);
    fputs("more\n", b);
    fclose(b);
    fclose(c);
    printf("missing %d\n", fopen("rts6x-m6-none.txt", "r") == 0);
    show("rts6x-m6-a.txt");
    show("rts6x-m6-b.txt");
    show("rts6x-m6-c.txt");
    return 0;
}
