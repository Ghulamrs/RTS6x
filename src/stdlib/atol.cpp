// Spec: ISO C 7.20.1.2 - atol.

#include <stdlib.h>
#include "NumberText.h"

extern "C" long atol(const char *s)
{
    return rts6x::NumberText::toLong(s, 0, 10);
}
