// Spec: ISO C++11 15.5.1 (an exception nothing catches calls terminate) and 18.8.3 (set_terminate:
// the handler runs; this one ends the program with exit's status).
#include <stdio.h>
#include <stdlib.h>
#include <exception>

static void last() { printf("terminate handler\n"); exit(3); }

int main()
{
    std::set_terminate(last);
    printf("throwing\n");
    throw 1;
}
