// Spec: ISO C 7.21.2.2 - memmove.

#include <string.h>
#include "Memory.h"

extern "C" void *memmove(void *to, const void *from, size_t n)
{
    return rts6x::Memory::move(to, from, n);
}
