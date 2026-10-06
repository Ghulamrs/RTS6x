// Spec: the C$$IO$$ host channel (vm6747sim src/C6xHost.cpp): open 0xF0 (flags at parameter 2, the
// path as data, the descriptor answered), close 0xF1, read 0xF2 (fd, count; the count and the bytes
// answered) and lseek 0xF4 (fd, a 32-bit offset at parameter 2, the origin at 6; the position).

#include "CioChannel.h"
#include "DescriptorModes.h"

namespace rts6x {

int CioChannel::open(const char *path, unsigned flags)
{
    unsigned char params[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    put16(params + 2, flags);
    requestText(Open, params, path, 0);
    int fd = answer16();
    DescriptorModes::opened(fd);
    return fd;
}

int CioChannel::close(int fd)
{
    unsigned char params[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    put16(params, (unsigned)fd);
    request(Close, params, 0, 0);
    int result = answer16();
    if (result == 0) DescriptorModes::closed(fd);
    return result;
}

int CioChannel::read(int fd, char *bytes, unsigned count)
{
    unsigned char params[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    if (count > DataCapacity) count = DataCapacity;
    put16(params, (unsigned)fd);
    put16(params + 2, count);
    request(Read, params, 0, 0);
    unsigned got = get16(_CIOBUF_ + 4);
    if (got == 0xFFFF) return -1;
    if (got > count) got = count;
    for (unsigned i = 0; i < got; i++) bytes[i] = (char)_CIOBUF_[ReplyHeader + i];
    return (int)got;
}

long CioChannel::seek(int fd, long offset, Origin origin)
{
    unsigned char params[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    put16(params, (unsigned)fd);
    put32(params + 2, (unsigned)offset);
    put16(params + 6, (unsigned)origin);
    request(Lseek, params, 0, 0);
    return (long)get32(_CIOBUF_ + 4);
}

}  // namespace rts6x
