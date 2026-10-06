// Spec: ISO C 7.19.1 (stdin, stdout, stderr); SPRAB89B 9.18 (they are &_ftable[0], [1], [2]).
// The table of FILE, with the three standard streams first on the host's descriptors 0, 1 and 2.
// Data alone - a translation unit with no function, which cpp11 takes since 2026-10-06.

#include <stdio.h>

FILE _ftable[20] = {
    { 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0 },
    { 2, 0, 0, 0, 0, 0 },
};
