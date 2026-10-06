// Spec: ISO C 7.19.6.1/14 (the count is of characters transmitted) and 7.19.6.6/2 (sprintf ends
// what it writes with a NUL) - the two destinations of OutputSink.h.
#include "OutputSink.h"
#include "../host/CioChannel.h"

namespace rts6x {

OutputSink::OutputSink(char *dst, size_t capacity)
    : dst_(dst), capacity_(capacity), count_(0), fd_(-1), failed_(false), staged_(0)
{
}

OutputSink::OutputSink(int fd)
    : dst_(0), capacity_(0), count_(0), fd_(fd), failed_(false), staged_(0)
{
}

void OutputSink::flush()
{
    if (staged_ != 0 && CioChannel::write(fd_, staging_, staged_) < 0) failed_ = true;
    staged_ = 0;
}

void OutputSink::put(char c)
{
    if (fd_ >= 0) {
        if (staged_ == Staging) flush();
        staging_[staged_++] = c;
    } else if (count_ + 1 < capacity_) {
        dst_[count_] = c;
    }
    count_++;
}

void OutputSink::put(const char *s, size_t n)
{
    for (size_t i = 0; i < n; i++) put(s[i]);
}

void OutputSink::repeat(char c, size_t n)
{
    for (size_t i = 0; i < n; i++) put(c);
}

int OutputSink::finish()
{
    if (fd_ >= 0) flush();
    else if (capacity_ != 0) dst_[count_ < capacity_ ? count_ : capacity_ - 1] = '\0';
    return failed_ ? -1 : (int)count_;
}

}  // namespace rts6x
