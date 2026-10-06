// Spec: Microsoft C runtime, _setmode - a descriptor's translation mode, _O_TEXT or _O_BINARY, the
// previous one answered. The C6747's host channel translates nothing, so the mode is only remembered;
// 0 marks a descriptor that is not open.
#ifndef RTS6X_DESCRIPTOR_MODES_H
#define RTS6X_DESCRIPTOR_MODES_H

namespace rts6x {

class DescriptorModes {
public:
    enum { Text = 0x4000, Binary = 0x8000, Count = 32 };
    // The mode now in force for fd, setting it to mode: the old one, or -1 with errno set.
    static int exchange(int fd, int mode);
    // The host channel's open and close say so here; 0, 1 and 2 are open from the start.
    static void opened(int fd) { if (fd >= 0 && fd < Count) modes_[fd] = Text; }
    static void closed(int fd) { if (fd >= 0 && fd < Count) modes_[fd] = 0; }

private:
    static int modes_[Count];
};

}  // namespace rts6x

#endif
