// Spec: ISO C 7.20.4 (atexit: at least 32 functions, called in reverse order of registration by
// exit; abort calls none of them) and the Itanium C++ ABI 3.3.5 (__cxa_atexit: a destructor and
// its object, in the same reverse order as atexit's). One class owns the list and the ending.
#ifndef RTS6X_TERMINATION_H
#define RTS6X_TERMINATION_H

extern "C" void __rts6x_halt(int status);

namespace rts6x {

class Termination {
public:
    // A function for exit to call; 0, or non-zero when the list is full.
    static int add(void (*function)(void));
    // A destructor and its object, for exit to call in the same list (C++).
    static int add(void (*destructor)(void *), void *object);
    // Calls the list, last first, then stops with status - which vm6747sim reports.
    static void exit(int status);
    // Stops at once, the list not called: 134, as a host shell reports a program that aborted.
    static void abort();

private:
    enum { Capacity = 64, AbortStatus = 134 };
    struct Entry {
        void (*plain)(void);
        void (*withObject)(void *);
        void *object;
    };
    static Entry entries_[Capacity];
    static int count_;
};

}  // namespace rts6x

#endif
