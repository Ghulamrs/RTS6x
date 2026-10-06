// Spec: ISO C 7.19.5.3 (fopen: r, w, a, each with + and b in either order), 7.19.5.1 (fclose) and
// 7.19.4.3 (tmpfile: removed when closed). The host's own flags say the same as the mode; a refusal
// sets errno, EINVAL for a mode C does not have, ENOENT for one the host turned down.

#include <stdlib.h>
#include <errno.h>
#include "Stream.h"
#include "../host/CioChannel.h"
#include "../misc/ErrorNumber.h"

namespace rts6x {

bool Stream::parseMode(const char *mode, unsigned &host, unsigned &own)
{
    bool update = false;
    for (const char *p = mode + 1; *p; p++) {
        if (*p == '+') update = true;
        else if (*p != 'b') return false;
    }
    switch (mode[0]) {
    case 'r': host = CioChannel::ReadOnly; own = Readable; break;
    case 'w': host = CioChannel::WriteOnly | CioChannel::Create | CioChannel::Truncate; own = Writable; break;
    case 'a': host = CioChannel::WriteOnly | CioChannel::Create | CioChannel::Append; own = Writable; break;
    default: return false;
    }
    if (update) {
        host = (host & ~3u) | CioChannel::ReadWrite;
        own = Readable | Writable;
    }
    host |= CioChannel::Binary;
    return true;
}

FILE *Stream::open(const char *path, const char *mode, FILE *slot)
{
    unsigned host, own;
    if (!parseMode(mode, host, own)) {
        ErrorNumber::set(EINVAL);
        return 0;
    }
    for (int i = FirstFree; !slot && i < RTS6X_FTABLE_COUNT; i++)
        if (!(_ftable[i].flags & Open)) slot = &_ftable[i];
    if (!slot) return 0;
    // The host gives no reason for a refusal; the commonest is a file that is not there.
    int fd = CioChannel::open(path, host);
    if (fd < 0) {
        ErrorNumber::set(ENOENT);
        return 0;
    }
    slot->fd = fd;
    slot->buf = slot->pos = slot->bufend = slot->buff_stop = 0;
    slot->flags = Open | own;
    return slot;
}

int Stream::close()
{
    if (!(f_->flags & Open)) return EOF;
    int r = f_->fd > 2 ? CioChannel::close(f_->fd) : 0;
    if (f_->flags & OwnBuffer) free(f_->buf);
    if (f_->flags & Temporary) {
        char name[24];
        temporaryName((int)(f_ - _ftable), name);
        CioChannel::unlink(name);
    }
    f_->buf = f_->pos = f_->bufend = f_->buff_stop = 0;
    f_->flags = 0;
    return r < 0 ? EOF : 0;
}

void Stream::temporaryName(int slot, char *name)
{
    const char *stem = "rts6x-tmp";
    int n = 0;
    while (stem[n]) { name[n] = stem[n]; n++; }
    name[n++] = (char)('0' + slot / 10);
    name[n++] = (char)('0' + slot % 10);
    name[n] = 0;
}

}  // namespace rts6x
