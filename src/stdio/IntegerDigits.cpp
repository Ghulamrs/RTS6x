// Spec: positional notation, base 8, 10 or 16. Ten does not divide a power of two, so a decimal
// digit comes from long division over the four 16-bit pieces; 8 and 16 are shifts of the pair.

#include "IntegerDigits.h"

namespace rts6x {

IntegerDigits::IntegerDigits(unsigned hi, unsigned lo, unsigned base, bool upper) : length_(0)
{
    const char *glyphs = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char *end = text_ + sizeof text_;
    while (hi != 0 || lo != 0) {
        unsigned d;
        if (base == 10) {
            unsigned piece[4] = { hi >> 16, hi & 0xFFFFu, lo >> 16, lo & 0xFFFFu };
            unsigned r = 0;
            for (int k = 0; k < 4; k++) {
                unsigned cur = r * 65536u + piece[k];
                piece[k] = cur / 10u;
                r = cur - piece[k] * 10u;
            }
            hi = (piece[0] << 16) | piece[1];
            lo = (piece[2] << 16) | piece[3];
            d = r;
        } else {
            unsigned bits = base == 16 ? 4 : 3;
            d = lo & (base - 1);
            lo = (lo >> bits) | (hi << (32 - bits));
            hi >>= bits;
        }
        *--end = glyphs[d];
        length_++;
    }
}

}  // namespace rts6x
