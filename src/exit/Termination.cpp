// Spec: ISO C 7.20.4.2-3 and Itanium C++ ABI 3.3.5: one list for atexit and __cxa_atexit,
// emptied from the end, so each function runs after everything registered later than it.

#include "Termination.h"

namespace rts6x {

Termination::Entry Termination::entries_[Termination::Capacity];
int Termination::count_;

int Termination::add(void (*function)(void))
{
    if (count_ == Capacity) return -1;
    entries_[count_].plain = function;
    entries_[count_].withObject = 0;
    entries_[count_].object = 0;
    count_++;
    return 0;
}

int Termination::add(void (*destructor)(void *), void *object)
{
    if (count_ == Capacity) return -1;
    entries_[count_].plain = 0;
    entries_[count_].withObject = destructor;
    entries_[count_].object = object;
    count_++;
    return 0;
}

void Termination::exit(int status)
{
    // A function the list calls may register another; it is called too, before those below it.
    while (count_ > 0) {
        Entry e = entries_[--count_];
        if (e.plain) e.plain();
        else e.withObject(e.object);
    }
    __rts6x_halt(status);
}

void Termination::abort()
{
    __rts6x_halt(AbortStatus);
}

}  // namespace rts6x
