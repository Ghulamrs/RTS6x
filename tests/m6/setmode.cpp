// Spec: Microsoft C runtime, _setmode - the previous mode for an open descriptor (stdout starts in
// text), -1 and EINVAL for a mode that is neither, -1 and EBADF for a descriptor that is not open.
// The expected output is cl's own: run on the Windows box with an invalid-parameter handler that returns.
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <stdlib.h>
static void keepGoing(const wchar_t *, const wchar_t *, const wchar_t *, unsigned, uintptr_t) {}
#endif

static void show(const char *what, int fd, int mode)
{
    errno = 0;
    int result = _setmode(fd, mode);
    printf("%s: %d errno %s\n", what, result, errno == EINVAL ? "EINVAL" : errno == EBADF ? "EBADF" : errno == 0 ? "0" : "other");
}

int main()
{
#ifdef _MSC_VER
    _set_invalid_parameter_handler(keepGoing);
#endif
    fflush(stdout);
    show("stdout binary", 1, _O_BINARY);
    show("stdout text", 1, _O_TEXT);
    show("stdout text again", 1, _O_TEXT);
    show("stderr binary", 2, _O_BINARY);
    show("mode 0x1234", 1, 0x1234);
    show("fd 7, not open", 7, _O_TEXT);
    show("fd -1", -1, _O_TEXT);
    FILE *f = fopen("setmode.tmp", "w");
    int fd = fileno(f);
    show("an opened file", fd, _O_BINARY);
    fclose(f);
    show("the same, closed", fd, _O_TEXT);
    remove("setmode.tmp");
    return 0;
}
