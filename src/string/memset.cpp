// Spec: ISO C 7.21.6.1 - memset.

#include <string.h>
#include "Memory.h"

extern "C" void *memset(void *to, int value, size_t n)
{
    return rts6x::Memory::fill(to, value, n);
}
