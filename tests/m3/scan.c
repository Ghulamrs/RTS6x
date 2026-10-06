/* Spec: ISO C 7.19.6.2 - sscanf's conversions and directives, the count assigned, %n, suppression,
   widths, scansets, and EOF on an empty input; scanf reading the program's standard input. */
#include <stdio.h>

int main(void)
{
    int a = 0, b = 0, n = 0, r;
    unsigned u = 0;
    long l = 0;
    short h = 0;
    char c = 0, s[32] = "", t[32] = "", set[32] = "";
    float f = 0;
    double d = 0;
    void *p = 0;

    r = sscanf("  12 -34 0x1f 017 0x10", "%d %i %x %o %hi", &a, &b, &u, &n, &h);
    printf("%d: %d %d %u %d %d\n", r, a, b, u, n, h);
    r = sscanf("123456 abc", "%3d%2d %s", &a, &b, s);
    printf("%d: %d %d %s\n", r, a, b, s);
    r = sscanf("key=value;rest", "%[^=]=%[a-z];%n", s, t, &n);
    printf("%d: [%s] [%s] %d\n", r, s, t, n);
    r = sscanf("x:  y", "%c:%*c%c", &c, &set[0]);
    printf("%d: %c [%c]\n", r, c, set[0]);
    r = sscanf("3.25 -1e-3 inf 0x1.8p1", "%f %lf %s %lf", &f, &d, s, &d);
    printf("%d: %g %g %s\n", r, (double)f, d, s);
    r = sscanf("1.5e", "%lf", &d);
    printf("partial exponent %d\n", r);
    r = sscanf("  42%", "%ld%%", &l);
    printf("%d: %ld\n", r, l);
    r = sscanf("abc", "%d", &a);
    printf("no digits %d\n", r);
    r = sscanf("", "%d", &a);
    printf("empty %d\n", r);
    r = sscanf("   ", "%s", s);
    printf("spaces %d\n", r);
    r = sscanf("0x80000000", "%p", &p);
    printf("%d: %d\n", r, p != 0);
    r = sscanf("]x-]", "%[]x-]", set);
    printf("%d: [%s]\n", r, set);
    r = sscanf("99 bottles", "%d %5c", &a, t);
    t[5] = 0;
    printf("%d: %d [%s]\n", r, a, t);

    r = scanf("%d %s", &a, s);
    printf("stdin %d: %d %s\n", r, a, s);
    r = scanf("%lf", &d);
    printf("stdin %d: %g\n", r, d);
    r = scanf("%d", &a);
    printf("stdin at end %d\n", r);
    return 0;
}
