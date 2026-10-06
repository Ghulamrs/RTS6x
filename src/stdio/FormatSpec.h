// Spec: ISO C 7.19.6.1/4-8 - one conversion specification: flags, field width, precision, length
// modifier and conversion specifier, either number of which may be `*`, taken from the arguments.
#ifndef RTS6X_FORMAT_SPEC_H
#define RTS6X_FORMAT_SPEC_H

#include <stdarg.h>
#include "FormatText.h"

namespace rts6x {

class FormatSpec {
public:
    enum Length { Plain, Char, Short, Long, LongLong, IntMax, Size, PtrDiff, LongDouble };

    FormatSpec();
    // Reads a specification from just after its '%'; false if it is not one C defines.
    bool parse(FormatText &text, va_list *args);

    bool leftAlign() const { return leftAlign_; }
    bool plusSign() const { return plusSign_; }
    bool spaceSign() const { return spaceSign_; }
    bool alternate() const { return alternate_; }
    bool zeroPad() const { return zeroPad_; }
    int width() const { return width_; }
    int precision() const { return precision_; }    // -1 when none is given
    Length length() const { return length_; }
    char conversion() const { return conversion_; }
    bool upper() const { return conversion_ >= 'A' && conversion_ <= 'Z'; }

private:
    static int digits(FormatText &text);

    bool leftAlign_, plusSign_, spaceSign_, alternate_, zeroPad_;
    int width_, precision_;
    Length length_;
    char conversion_;
};

}  // namespace rts6x

#endif
