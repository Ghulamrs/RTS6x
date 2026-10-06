// Spec: SPRAB89B 11.2 and the ARM EHABI 6 (the index of functions; PREL31 offsets, which
// the C6000 counts in halfwords) and 11.4 (the personality's words: PR3's compact one with the
// pop mask, PR2's byte codes - MV FP,SP; POP mask; RETURN - with a count of words that follow).

#include "UnwindTable.h"
#include "UnwindContext.h"

extern "C" {
extern const char __TI_UNWIND_TABLE_START[];
extern const char __TI_UNWIND_TABLE_END[];
}

namespace rts6x {

namespace {

unsigned word(unsigned at) { return *reinterpret_cast<const unsigned *>(at); }

}  // namespace

unsigned UnwindEntry::prel31(unsigned place)
{
    unsigned v = word(place) & 0x7fffffffu;
    if (v & 0x40000000u) v |= 0x80000000u;
    return place + (v << 1);
}

bool UnwindEntry::find(unsigned pc, UnwindEntry &entry)
{
    unsigned lo = reinterpret_cast<unsigned>(__TI_UNWIND_TABLE_START);
    unsigned hi = reinterpret_cast<unsigned>(__TI_UNWIND_TABLE_END);
    // The entry starting nearest below pc: a scan, not a search, since lnk6x leaves an entry for each
    // discarded copy of a weak function, naming the kept copy, where the discarded one would have been.
    unsigned at = 0, best = 0;
    for (unsigned e = lo; e < hi; e += 8) {
        unsigned start = prel31(e);
        if (start <= pc - 1 && (at == 0 || start >= best)) {
            at = e;
            best = start;
        }
    }
    if (at == 0) return false;
    unsigned second = word(at + 4);
    if (second == 1) return false;
    entry.function_ = prel31(at);
    entry.table_ = (second & 0x80000000u) ? 0 : prel31(at + 4);
    entry.word_ = entry.table_ ? word(entry.table_) : second;
    entry.wide_ = (entry.word_ >> 24) == 0x82;
    entry.descriptors_ = 0;
    if (entry.table_) entry.descriptors_ = entry.table_ + 4 + (entry.wide_ ? 4 * (entry.word_ >> 16 & 0xff) : 0);
    return true;
}

bool UnwindEntry::frameMask(unsigned &mask) const
{
    if ((word_ >> 24) == 0x83) {
        if ((word_ >> 17 & 0x7f) != 0x7f) return false;
        mask = word_ >> 4 & 0x1fff;
        return true;
    }
    if (!wide_ || !table_) return false;
    // PR2: the first two codes in the word, the rest in the words after it, high byte first.
    unsigned op[4];
    op[0] = word_ >> 8 & 0xff;
    op[1] = word_ & 0xff;
    unsigned more = word_ >> 16 & 0xff;
    if (more == 0) return false;
    unsigned next = word(table_ + 4);
    op[2] = next >> 24 & 0xff;
    op[3] = next >> 16 & 0xff;
    if (op[0] != 0xd0 || (op[1] & 0xe0) != 0x80 || op[3] != 0xe7) return false;
    mask = (op[1] & 0x1f) << 8 | op[2];
    return true;
}

bool UnwindEntry::unwind(UnwindContext &context, unsigned &pc) const
{
    unsigned mask;
    if (!frameMask(mask) || (mask & 1u << 12) == 0 || (mask & 1u << 5) == 0) return false;
    // The save area below the frame pointer, a word a register from bit 12 down; A15 at fp itself.
    static const signed char slotOfBit[13] = {
        UnwindContext::A10, UnwindContext::A11, UnwindContext::A12, UnwindContext::A13, UnwindContext::A14, -1,
        UnwindContext::B10, UnwindContext::B11, UnwindContext::B12, UnwindContext::B13, UnwindContext::B14, -1, -1 };
    unsigned fp = context.frame(), at = 0, returnAddress = 0;
    for (int bit = 12; bit >= 0; bit--) {
        if ((mask & 1u << bit) == 0) continue;
        unsigned value = word(fp - 4 * at++);
        if (bit == 5) returnAddress = value;
        else if (slotOfBit[bit] >= 0) context.set((UnwindContext::Slot)slotOfBit[bit], value);
    }
    context.set(UnwindContext::B15, fp);
    context.set(UnwindContext::A15, word(fp));
    pc = returnAddress;
    return true;
}

bool ScopeDescriptor::read(unsigned at, const UnwindEntry &entry)
{
    if (at == 0 || word(at) == 0) return false;
    const bool wide = entry.wide();
    const unsigned w = wide ? 4 : 0;
    unsigned length, offset;
    if (wide) {
        length = word(at);
        offset = word(at + 4);
    } else {
        unsigned both = word(at);
        length = both & 0xffff;
        offset = both >> 16;
    }
    kind_ = (Kind)(((length & 1) << 1) | (offset & 1));
    begin_ = entry.function() + (offset & ~1u);
    end_ = begin_ + (length & ~1u);
    pad_ = 0;
    type_ = 0;
    count_ = 0;
    if (kind_ == Specification) {
        count_ = word(at + 4 + w);
        next_ = at + 8 + w + 4 * (count_ & 0x7fffffffu) + ((count_ & 0x80000000u) ? 4 : 0);
        return true;
    }
    pad_ = UnwindEntry::prel31(at + 4 + w);
    if (kind_ == Catch) {
        unsigned t = word(at + 8 + w);
        type_ = t;
        next_ = at + 12 + w;
    } else {
        next_ = at + 8 + w;
    }
    return true;
}

}  // namespace rts6x
