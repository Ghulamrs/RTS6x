// Spec: ISO C 7.21.4.2 - strcmp.

#include <string.h>
#include "CString.h"

extern "C" int strcmp(const char *a, const char *b)
{
    return rts6x::CString::compare(a, b);
}
