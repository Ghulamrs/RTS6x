// Spec: SPRAB89B 11.5 and the ARM EHABI 8.4 - __cxa_end_cleanup ends a cleanup and resumes the
// unwinding that reached it. Until M5 nothing unwinds, so no cleanup is entered; were one to be,
// the program ends.

extern "C" void __c6xabi_abort_msg(const char *message);

extern "C" void __cxa_end_cleanup(void)
{
    __c6xabi_abort_msg("__cxa_end_cleanup reached with no unwinder\n");
}
