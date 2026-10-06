// Spec: Itanium C++ ABI 2.5.3 - __cxa_call_unexpected, reached when an exception leaves a function
// whose specification does not allow it. Until M5 nothing throws; were it reached, the program ends.

extern "C" void __c6xabi_abort_msg(const char *message);

extern "C" void __cxa_call_unexpected(void *exception)
{
    (void)exception;
    __c6xabi_abort_msg("an exception violated a dynamic exception specification\n");
}
