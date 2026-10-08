// Spec: ISO C 7.23.2.3 - mktime at the edges of a 32-bit time_t: the last and first second it holds,
// and the seconds beyond them, which it has no value for. The host's time_t is wider, so a result past
// the edge is printed as "beyond" whether it is the wider value or -1.
#include <stdio.h>
#include <string.h>
#include <time.h>

static void at(int year, int mon, int mday, int hour, int min, int sec)
{
    struct tm t;
    memset(&t, 0, sizeof t);
    t.tm_year = year - 1900; t.tm_mon = mon - 1; t.tm_mday = mday; t.tm_hour = hour; t.tm_min = min; t.tm_sec = sec;
    time_t r = mktime(&t);
    long long v = (long long)r;
    if (v >= -2147483647LL - 1 && v <= 2147483647LL && !(v == -1 && year != 1969))
        printf("%d-%02d-%02d %02d:%02d:%02d = %lld, wday %d yday %d\n", year, mon, mday, hour, min, sec, v, t.tm_wday, t.tm_yday);
    else
        printf("%d-%02d-%02d %02d:%02d:%02d beyond\n", year, mon, mday, hour, min, sec);
}

int main()
{
    at(2038, 1, 19, 3, 14, 7);
    at(2038, 1, 19, 3, 14, 8);
    at(2038, 1, 19, 0, 0, 0);
    at(2038, 1, 20, 0, 0, 0);
    at(1901, 12, 13, 20, 45, 52);
    at(1901, 12, 13, 20, 45, 51);
    at(1901, 12, 14, 0, 0, 0);
    at(1901, 12, 13, 0, 0, 0);
    at(1969, 12, 31, 23, 59, 59);
    at(2000, 2, 29, 12, 0, 0);
    at(2038, 1, 18, 27, 14, 7);
    return 0;
}
