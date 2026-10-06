// Spec: ISO C 7.19.6.10 - vprintf is printf with the arguments already in a va_list.

#include <stdio.h>
#include <stdarg.h>
#include "Formatter.h"

extern "C" int vprintf(const char *format, va_list args)
{
    rts6x::OutputSink out(1);
    return rts6x::Formatter(out, &args).run(rts6x::FormatText(format));
}
