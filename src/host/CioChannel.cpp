// Spec: the C$$IO$$ host channel, its write command (0xF3): parameters fd:2 and count:2, and the
// answer's first parameter the count written, 0xFFFF for a refusal (vm6747sim src/C6xHost.cpp).

#include "CioChannel.h"

namespace rts6x {

void CioChannel::request(Command c, const unsigned char params[8], const char *data, unsigned length)
{
    unsigned char *b = _CIOBUF_;
    put32(b, length);
    b[4] = (unsigned char)c;
    for (unsigned i = 0; i < 8; i++) b[5 + i] = params[i];
    for (unsigned i = 0; i < length; i++) b[RequestHeader + i] = (unsigned char)data[i];
    __rts6x_cio_trap();
}

int CioChannel::write(int fd, const char *bytes, unsigned count)
{
    unsigned done = 0;
    while (done < count) {
        unsigned chunk = count - done < DataCapacity ? count - done : (unsigned)DataCapacity;
        unsigned char params[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        put16(params, (unsigned)fd);
        put16(params + 2, chunk);
        request(Write, params, bytes + done, chunk);
        unsigned wrote = get16(_CIOBUF_ + 4);
        if (wrote == 0xFFFF || wrote == 0) return -1;
        done += wrote;
    }
    return (int)done;
}

}  // namespace rts6x
