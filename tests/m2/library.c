/* Spec: none - M2's C library against the host's: string.h, ctype.h over every unsigned char,
   strtol and strtoul (errno and ERANGE at the ends), qsort and bsearch, abs, vsprintf,
   and rand against C's own example generator (7.20.2.2/5). */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>

static int byInt(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return x < y ? -1 : x > y; }
static int byText(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static int formatted(char *out, const char *format, ...)
{
    va_list args;
    int n;
    va_start(args, format);
    n = vsprintf(out, format, args);
    va_end(args);
    return n;
}

int main(void)
{
    char buf[64], *end;
    char text[] = "alpha,beta;;gamma";
    char *tok;
    int nums[] = { 42, -7, 0, 99, 13, -7, 1000, 5 };
    const char *words[] = { "pear", "apple", "fig", "kiwi", "banana" };
    int key = 13, i, sum, *found;
    unsigned c, classes[12] = { 0 };
    unsigned next = 1;

    strcpy(buf, "hello");
    strcat(buf, ", world");
    printf("[%s] %d %d %d %d\n", buf, (int)strlen(buf), strcmp("abc", "abd") < 0, strncmp("abcx", "abcy", 3), strcmp("b", "a") > 0);
    printf("[%s] [%s] [%s] [%s]\n", strchr(buf, 'o'), strrchr(buf, 'o'), strstr(buf, "wor"), strpbrk(buf, ",w"));
    printf("%d %d %d %d\n", (int)strspn("aabbc", "ab"), (int)strcspn("xyz;q", ";"), strchr(buf, 0) == buf + strlen(buf), strstr(buf, "") == buf);
    strncpy(buf, "ab", 5);
    printf("strncpy pads: %d %d %d\n", buf[2], buf[3], buf[4]);
    for (tok = strtok(text, ",;"); tok; tok = strtok(0, ",;")) printf("token [%s]\n", tok);
    memset(buf, 'x', 10); buf[10] = 0;
    memmove(buf + 2, buf, 5);
    memcpy(buf, "AB", 2);
    printf("[%s] %d %d [%s]\n", buf, memcmp("ab", "ac", 2) < 0, memcmp("ab", "ab", 2), (char *)memchr("lookup", 'k', 6));

    for (c = 0; c < 256; c++) {
        classes[0] += isalnum(c) != 0; classes[1] += isalpha(c) != 0; classes[2] += iscntrl(c) != 0;
        classes[3] += isdigit(c) != 0; classes[4] += isgraph(c) != 0; classes[5] += islower(c) != 0;
        classes[6] += isprint(c) != 0; classes[7] += ispunct(c) != 0; classes[8] += isspace(c) != 0;
        classes[9] += isupper(c) != 0; classes[10] += isxdigit(c) != 0; classes[11] += tolower(c) + toupper(c);
    }
    for (i = 0; i < 12; i++) printf("%u%c", classes[i], i == 11 ? '\n' : ' ');

    printf("%ld %ld %ld %ld\n", strtol("  -1234xyz", &end, 10), strtol("0x1f", 0, 16), strtol("0777", 0, 0), strtol("z", 0, 36));
    printf("[%s] %lu %lu %d %d\n", end, strtoul("4294967295", 0, 10), strtoul("ff", 0, 16), atoi("  42abc"), (int)atol("-99"));
    errno = 0;
    printf("overflow: %d %d ", strtol("99999999999999999999", 0, 10) == LONG_MAX, errno == ERANGE);
    errno = 0;
    printf("%d %d ", strtol("-99999999999999999999", 0, 10) == LONG_MIN, errno == ERANGE);
    end = 0;
    printf("none: %ld %d\n", strtol("   +x", &end, 10), *end == ' ');

    qsort(nums, 8, sizeof(int), byInt);
    for (i = 0; i < 8; i++) printf("%d ", nums[i]);
    qsort((void *)words, 5, sizeof(char *), byText);
    for (i = 0; i < 5; i++) printf("%s ", words[i]);
    found = (int *)bsearch(&key, nums, 8, sizeof(int), byInt);
    printf("| found %d at %d, %d %d\n", *found, (int)(found - nums), abs(-5), (int)labs(-6L));

    printf("vsprintf %d [%s]\n", formatted(buf, "<%05.1f|%x>", 3.14159, 255), buf);

    sum = 0;
#ifdef __TMS320C6X__
    srand(1);
    for (i = 0; i < 5; i++) {
        next = next * 1103515245u + 12345u;
        sum += rand() == (int)((next / 65536u) % 32768u);
    }
#else
    sum = 5;
#endif
    printf("rand follows C's example: %d of 5\n", sum);
    return 0;
}
