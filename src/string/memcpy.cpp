// Spec: ISO C 7.21.2.1 - memcpy.

#include <string.h>
#include "Memory.h"

extern "C" void *memcpy(void *to, const void *from, size_t n)
{
    return rts6x::Memory::copy(to, from, n);
}
