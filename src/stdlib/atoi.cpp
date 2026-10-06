// Spec: ISO C 7.20.1.2 - atoi.

#include <stdlib.h>
#include "NumberText.h"

extern "C" int atoi(const char *s)
{
    return (int)rts6x::NumberText::toLong(s, 0, 10);
}
