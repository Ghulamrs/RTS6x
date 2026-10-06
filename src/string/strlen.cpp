// Spec: ISO C 7.21.6.3 - strlen.

#include <string.h>
#include "CString.h"

extern "C" size_t strlen(const char *s)
{
    return rts6x::CString::length(s);
}
