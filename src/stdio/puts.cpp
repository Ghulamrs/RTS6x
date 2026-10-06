// Spec: ISO C 7.19.7.10 - puts writes the string and a new-line to stdout; non-negative, or EOF.

#include <stdio.h>
#include "OutputSink.h"

extern "C" int puts(const char *s)
{
    rts6x::OutputSink out(1);
    out.put(s);
    out.put('\n');
    return out.finish() < 0 ? EOF : 0;
}
