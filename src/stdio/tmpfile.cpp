// Spec: ISO C 7.19.4.3 - tmpfile: a file opened "wb+", removed when it is closed or the program ends.

#include "Stream.h"
#include "../exit/Termination.h"

namespace {

void closeTemporaries()
{
    for (int i = rts6x::Stream::FirstFree; i < RTS6X_FTABLE_COUNT; i++)
        if (_ftable[i].flags & rts6x::Stream::Temporary) {
            rts6x::Stream s(&_ftable[i]);
            s.close();
        }
}

}  // namespace

extern "C" FILE *tmpfile(void)
{
    static bool registered = false;
    for (int i = rts6x::Stream::FirstFree; i < RTS6X_FTABLE_COUNT; i++) {
        if (_ftable[i].flags & rts6x::Stream::Open) continue;
        char name[24];
        rts6x::Stream::temporaryName(i, name);
        FILE *f = rts6x::Stream::open(name, "wb+", &_ftable[i]);
        if (!f) return 0;
        f->flags |= rts6x::Stream::Temporary;
        if (!registered) registered = rts6x::Termination::add(closeTemporaries) == 0;
        return f;
    }
    return 0;
}
