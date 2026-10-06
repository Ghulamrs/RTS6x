// Spec: ISO C 7.21.5.2 - strchr.

#include <string.h>
#include "CString.h"

extern "C" char *strchr(const char *s, int c)
{
    return rts6x::CString::find(s, c);
}
