// Spec: ISO C 7.19.6.3 - printf is fprintf to stdout, whose descriptor is 1.
#include <stdio.h>
#include <stdarg.h>
#include "Formatter.h"

extern "C" int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    rts6x::OutputSink out(1);
    int n = rts6x::Formatter(out, &args).run(rts6x::FormatText(format));
    va_end(args);
    return n;
}
