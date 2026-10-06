// Spec: ISO C 7.21.2-7.21.6 - the mem* functions: copy (no overlap), move (overlap allowed), fill,
// compare as unsigned char, find a byte. A word at a time where both ends are word-aligned.
#ifndef RTS6X_MEMORY_H
#define RTS6X_MEMORY_H

#include <stddef.h>

namespace rts6x {

class Memory {
public:
    static void *copy(void *to, const void *from, size_t n);
    static void *move(void *to, const void *from, size_t n);
    static void *fill(void *to, int value, size_t n);
    static int compare(const void *a, const void *b, size_t n);
    static void *find(const void *in, int value, size_t n);

private:
    static bool aligned(const void *a, const void *b) { return (((size_t)a | (size_t)b) & 3) == 0; }
};

}  // namespace rts6x

#endif
