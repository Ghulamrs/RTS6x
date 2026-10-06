// Spec: ISO C 7.20.1.4 - strtol.

#include <stdlib.h>
#include "NumberText.h"

extern "C" long strtol(const char *s, char **end, int base)
{
    return rts6x::NumberText::toLong(s, end, base);
}
