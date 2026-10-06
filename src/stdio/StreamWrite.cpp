// Spec: ISO C 7.19.5.3/6 - on a stream opened for update, output may follow input only after a
// positioning call; RTS6x does the repositioning itself, giving the host back what was read ahead.

#include "Stream.h"
#include "../host/CioChannel.h"

namespace rts6x {

void Stream::giveBack()
{
    long ahead = (long)(f_->bufend - f_->pos);
    if (ahead) CioChannel::seek(f_->fd, -ahead, CioChannel::FromHere);
    f_->pos = f_->bufend = f_->buf;
}

int Stream::writeDescriptor()
{
    if (!(f_->flags & Writable)) {
        f_->flags |= Failed;
        return -1;
    }
    if (f_->pos < f_->bufend) giveBack();
    return f_->fd;
}

}  // namespace rts6x
