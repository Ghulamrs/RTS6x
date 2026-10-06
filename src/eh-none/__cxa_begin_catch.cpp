// Spec: Itanium C++ ABI 2.5.3 - __cxa_begin_catch, entered by a handler's landing pad - here, by
// the terminate scope cpp11 writes round a function declared throw(). Until M5 nothing unwinds to
// a landing pad; were one reached, the program ends.

extern "C" void __c6xabi_abort_msg(const char *message);

extern "C" void *__cxa_begin_catch(void *exception)
{
    (void)exception;
    __c6xabi_abort_msg("__cxa_begin_catch reached with no unwinder\n");
    return 0;
}
