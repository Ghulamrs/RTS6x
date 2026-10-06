// Spec: the C$$IO$$ host channel (vm6747sim src/C6xHost.cpp): a request in _CIOBUF_ is
// [length:4][command:1][parameters:8][data], the answer [length:4][parameters:8][data], little-endian.
// One class owns the buffer; the C library's file calls go through it and nothing else touches it.
#ifndef RTS6X_CIO_CHANNEL_H
#define RTS6X_CIO_CHANNEL_H

extern "C" {
extern unsigned char _CIOBUF_[];
void __rts6x_cio_trap(void);
}

namespace rts6x {

class CioChannel {
public:
    // The commands the host answers, in its numbering.
    enum Command { Open = 0xF0, Close = 0xF1, Read = 0xF2, Write = 0xF3, Lseek = 0xF4,
                   Unlink = 0xF5, Getenv = 0xF6, Rename = 0xF7, GetTime = 0xF8, GetClock = 0xF9 };
    // The most data one request carries; _CIOBUF_ is this plus the two headers, rounded up.
    enum { DataCapacity = 1024 };

    // count bytes to host descriptor fd, in as many requests as it takes: the number, or -1.
    static int write(int fd, const char *bytes, unsigned count);

private:
    enum { RequestHeader = 13, ReplyHeader = 12 };
    static void put16(unsigned char *p, unsigned v) { p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8); }
    static void put32(unsigned char *p, unsigned v) { put16(p, v); put16(p + 2, v >> 16); }
    static unsigned get16(const unsigned char *p) { return p[0] | (p[1] << 8); }
    // One request: the header and the data copied into _CIOBUF_, then the trap; the answer is
    // left in the buffer for the caller to read.
    static void request(Command c, const unsigned char params[8], const char *data, unsigned length);
};

}  // namespace rts6x

#endif
