// Spec: ISO C 7.20.5 - qsort sorts n objects of a size by the comparison, ascending; bsearch finds
// one in an array so sorted, or null. qsort is a heapsort here: in place, no allocation.
#ifndef RTS6X_SORTING_H
#define RTS6X_SORTING_H

#include <stddef.h>

namespace rts6x {

class Sorting {
public:
    typedef int (*Compare)(const void *, const void *);
    static void sort(void *base, size_t n, size_t size, Compare compare);
    static void *search(const void *key, const void *base, size_t n, size_t size, Compare compare);

private:
    static void swap(char *a, char *b, size_t size);
    // Sinks element i of the heap of the first n elements into place.
    static void sink(char *base, size_t i, size_t n, size_t size, Compare compare);
};

}  // namespace rts6x

#endif
