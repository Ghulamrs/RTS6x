/* Spec: ISO C 7.20.1.3 and F.5 - strtod correctly rounded: halfway cases, long digit strings,
   subnormals, the edges of the range, hex forms, INF and NAN, and where the conversion stops. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void show(const char *s)
{
    char *end;
    double d;
    unsigned w[2];
    errno = 0;
    d = strtod(s, &end);
    memcpy(w, &d, sizeof d);
    printf("%-44.44s %08x%08x stop %d%s\n", s, w[1], w[0], (int)(end - s), errno == ERANGE ? " ERANGE" : "");
}

int main(void)
{
    show("0.1");
    show("1e23");
    show("8.98846567431158e307");
    show("1.7976931348623157e308");
    show("1.7976931348623158e308");
    show("1.7976931348623159e308");
    show("2e308");
    show("4.9406564584124654e-324");
    show("2.4703282292062327e-324");
    show("2.4703282292062328e-324");
    show("2.2250738585072011e-308");
    show("2.2250738585072014e-308");
    show("1e-400");
    show("9007199254740993");
    show("9007199254740992.9999999999999999999999999999");
    show("9007199254740993.0000000000000000000000000001");
    show("0.500000000000000166533453693773481063544750213623046875");
    show("3.14159265358979323846264338327950288419716939937510582097494");
    show("123456789012345678901234567890e-20");
    show("0x1.fffffffffffffp1023");
    show("0x1p-1074");
    show("0X.8P1");
    show("-0");
    show("  +.5e+1x");
    show("1e");
    show("1e+");
    show(".e1");
    show("infinity and");
    show("-INF");
    show("nan(123)x");
    show("nan(");
    show("0x");
    show("00000000000000000000000000000000000000000001");
    printf("atof %g\n", atof(" 6.25e2"));
    return 0;
}
