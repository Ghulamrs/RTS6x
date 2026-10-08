/* Spec: ISO C 7.23 - gmtime and mktime over the Gregorian calendar, strftime's conversions in the
   "C" locale, asctime, and strftime's 0 for a buffer no weekday name fits (7.23.3.5/3); time and
   clock answer something. Run against a host set to UTC. */
#include <stdio.h>
#include <time.h>
#include <string.h>

static void show(const struct tm *t)
{
    char text[256];
    size_t n = strftime(text, sizeof text,
        "%a %A %b %B %C %d %D %e %F %g %G %h %H %I %j %m %M %n%p %r %R %S %t%T %u %U %V %w %W %x %X %y %Y %Z %%", t);
    printf("%d [%s]\n", (int)n, text);
}

int main(void)
{
    static const long moments[] = { 0L, 68169600L, 951782400L, 1009843199L, 1700000000L, -86400L, 2147483647L };
    struct tm t;
    char small[4];
    int i;
    time_t now;
    for (i = 0; i < (int)(sizeof moments / sizeof moments[0]); i++) {
        time_t m = (time_t)moments[i];
        struct tm *g = gmtime(&m);
        printf("%ld: %d-%d-%d %d:%d:%d wday %d yday %d\n", moments[i], g->tm_year, g->tm_mon, g->tm_mday,
               g->tm_hour, g->tm_min, g->tm_sec, g->tm_wday, g->tm_yday);
        show(g);
        printf("asctime %s", asctime(g));
    }
    memset(&t, 0, sizeof t);
    t.tm_year = 99; t.tm_mon = 13; t.tm_mday = 31; t.tm_hour = 25; t.tm_min = -5; t.tm_sec = 70;
    printf("mktime %ld -> %d-%d-%d %d:%d:%d wday %d yday %d\n", (long)mktime(&t), t.tm_year, t.tm_mon,
           t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, t.tm_wday, t.tm_yday);
    printf("too small %d\n", (int)strftime(small, sizeof small, "%A", &t));
    now = time(0);
    printf("time %d, clock %d, difftime %g\n", now > 1700000000L, clock() >= 0, difftime(10, 4));
    return 0;
}
