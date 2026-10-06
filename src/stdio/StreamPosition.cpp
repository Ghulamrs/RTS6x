// Spec: ISO C 7.19.9.2 (fseek: SEEK_SET, SEEK_CUR, SEEK_END; the end-of-file indicator cleared,
// pushed-back characters dropped) and 7.19.9.4 (ftell: the position, or -1L). The host keeps the
// position; the stream's read-ahead is what lies between the host's and the program's.

#include "Stream.h"
#include "../host/CioChannel.h"

namespace rts6x {

int Stream::seek(long offset, int whence)
{
    if (whence < SEEK_SET || whence > SEEK_END) return -1;
    if (whence == SEEK_CUR) offset -= (long)(f_->bufend - f_->pos);
    f_->pos = f_->bufend = f_->buf;
    if (CioChannel::seek(f_->fd, offset, (CioChannel::Origin)whence) < 0) return -1;
    f_->flags &= ~(unsigned)AtEnd;
    return 0;
}

long Stream::tell()
{
    long host = CioChannel::seek(f_->fd, 0, CioChannel::FromHere);
    return host < 0 ? -1L : host - (long)(f_->bufend - f_->pos);
}

}  // namespace rts6x
