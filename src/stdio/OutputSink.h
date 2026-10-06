// Spec: ISO C 7.19.6.1/14 and 7.19.6.6/2 - what the printf family counts and where its output goes:
// a host file descriptor, staged and written through CioChannel, or memory, with sprintf's NUL.
// One class for both, so the formatter writes characters and never knows which.
#ifndef RTS6X_OUTPUT_SINK_H
#define RTS6X_OUTPUT_SINK_H

#include <stddef.h>

namespace rts6x {

class OutputSink {
public:
    // Into memory at dst; capacity is how many bytes may be written, the NUL included.
    OutputSink(char *dst, size_t capacity);
    // To the host file descriptor fd.
    explicit OutputSink(int fd);

    void put(char c);
    void put(const char *s, size_t n);
    void repeat(char c, size_t n);
    // Staged bytes written or the NUL stored; the count, or -1 if the host refused a write.
    int finish();

    size_t count() const { return count_; }

private:
    enum { Staging = 128 };
    void flush();

    char *dst_;
    size_t capacity_;
    size_t count_;
    int fd_;
    bool failed_;
    unsigned staged_;
    char staging_[Staging];
};

}  // namespace rts6x

#endif
