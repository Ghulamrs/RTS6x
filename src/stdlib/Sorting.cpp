// Spec: ISO C 7.20.5.1-2. A max-heap over the array, then its largest element swapped to the end
// and the heap rebuilt over the rest, n - 1 times.

#include "Sorting.h"

namespace rts6x {

void Sorting::swap(char *a, char *b, size_t size)
{
    for (size_t k = 0; k < size; k++) { char t = a[k]; a[k] = b[k]; b[k] = t; }
}

void Sorting::sink(char *base, size_t i, size_t n, size_t size, Compare compare)
{
    for (;;) {
        size_t largest = i, left = 2 * i + 1, right = left + 1;
        if (left < n && compare(base + left * size, base + largest * size) > 0) largest = left;
        if (right < n && compare(base + right * size, base + largest * size) > 0) largest = right;
        if (largest == i) return;
        swap(base + i * size, base + largest * size, size);
        i = largest;
    }
}

void Sorting::sort(void *base, size_t n, size_t size, Compare compare)
{
    char *b = static_cast<char *>(base);
    if (n < 2 || size == 0) return;
    for (size_t i = n / 2; i-- > 0;) sink(b, i, n, size, compare);
    for (size_t end = n - 1; end > 0; end--) {
        swap(b, b + end * size, size);
        sink(b, 0, end, size, compare);
    }
}

void *Sorting::search(const void *key, const void *base, size_t n, size_t size, Compare compare)
{
    const char *b = static_cast<const char *>(base);
    size_t low = 0, high = n;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        int c = compare(key, b + middle * size);
        if (c == 0) return (void *)(b + middle * size);
        if (c < 0) high = middle;
        else low = middle + 1;
    }
    return 0;
}

}  // namespace rts6x
