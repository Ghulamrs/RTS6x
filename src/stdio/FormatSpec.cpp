// Spec: ISO C 7.19.6.1/4-8: flags `- + space # 0`; width as digits or `*` (negative: `-` and its
// magnitude); precision as `.` then digits or `*` (negative: none); `hh h l ll j z t L`.

#include "FormatSpec.h"

namespace rts6x {

FormatSpec::FormatSpec()
    : leftAlign_(false), plusSign_(false), spaceSign_(false), alternate_(false), zeroPad_(false),
      width_(0), precision_(-1), length_(Plain), conversion_(0)
{
}

int FormatSpec::digits(FormatText &text)
{
    int n = 0;
    while (text.at() >= '0' && text.at() <= '9') {
        n = n * 10 + (int)(text.at() - '0');
        text.next();
    }
    return n;
}

bool FormatSpec::parse(FormatText &text, va_list *args)
{
    for (;; text.next()) {
        unsigned c = text.at();
        if (c == '-') leftAlign_ = true;
        else if (c == '+') plusSign_ = true;
        else if (c == ' ') spaceSign_ = true;
        else if (c == '#') alternate_ = true;
        else if (c == '0') zeroPad_ = true;
        else break;
    }
    if (text.at() == '*') {
        text.next();
        width_ = va_arg(*args, int);
        if (width_ < 0) { leftAlign_ = true; width_ = -width_; }
    } else {
        width_ = digits(text);
    }
    if (text.at() == '.') {
        text.next();
        if (text.at() == '*') {
            text.next();
            precision_ = va_arg(*args, int);
            if (precision_ < 0) precision_ = -1;
        } else {
            precision_ = digits(text);
        }
    }
    unsigned c = text.at();
    if (c == 'h') { text.next(); length_ = Short; if (text.at() == 'h') { text.next(); length_ = Char; } }
    else if (c == 'l') { text.next(); length_ = Long; if (text.at() == 'l') { text.next(); length_ = LongLong; } }
    else if (c == 'j') { text.next(); length_ = IntMax; }
    else if (c == 'z') { text.next(); length_ = Size; }
    else if (c == 't') { text.next(); length_ = PtrDiff; }
    else if (c == 'L') { text.next(); length_ = LongDouble; }

    c = text.at();
    const char *known = "diouxXcspnfFeEgGaA";
    for (const char *k = known; *k; k++)
        if ((unsigned)(unsigned char)*k == c) {
            conversion_ = *k;
            text.next();
            return true;
        }
    return false;
}

}  // namespace rts6x
