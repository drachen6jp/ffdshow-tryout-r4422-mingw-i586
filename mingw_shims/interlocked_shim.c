// interlocked_shim.c - supply the five Interlocked* import cells that
// mingw-w64's libkernel32.a deliberately omits (its kernel32.def marks
// the decorated stdcall entries "DATA ; FIXME: why is decorated stdcall
// function symbol disabled?"). TUs that see the dllimport declarations
// in winbase.h call through __imp__ pointers; define those pointers
// here, aimed at local implementations built on __sync builtins.
#include <windows.h>

LONG WINAPI impl_InterlockedCompareExchange(LONG volatile *Destination, LONG Exchange, LONG Comparand)
__asm__("_InterlockedCompareExchange@12");
LONG WINAPI impl_InterlockedCompareExchange(LONG volatile *Destination, LONG Exchange, LONG Comparand)
{
    return __sync_val_compare_and_swap(Destination, Comparand, Exchange);
}

LONG WINAPI impl_InterlockedExchange(LONG volatile *Target, LONG Value)
__asm__("_InterlockedExchange@8");
LONG WINAPI impl_InterlockedExchange(LONG volatile *Target, LONG Value)
{
    return __sync_lock_test_and_set(Target, Value);
}

LONG WINAPI impl_InterlockedExchangeAdd(LONG volatile *Addend, LONG Value)
__asm__("_InterlockedExchangeAdd@8");
LONG WINAPI impl_InterlockedExchangeAdd(LONG volatile *Addend, LONG Value)
{
    return __sync_fetch_and_add(Addend, Value);
}

LONG WINAPI impl_InterlockedIncrement(LONG volatile *Addend)
__asm__("_InterlockedIncrement@4");
LONG WINAPI impl_InterlockedIncrement(LONG volatile *Addend)
{
    return __sync_add_and_fetch(Addend, 1);
}

LONG WINAPI impl_InterlockedDecrement(LONG volatile *Addend)
__asm__("_InterlockedDecrement@4");
LONG WINAPI impl_InterlockedDecrement(LONG volatile *Addend)
{
    return __sync_sub_and_fetch(Addend, 1);
}

void *imp_InterlockedCompareExchange __asm__("__imp___InterlockedCompareExchange@12") = (void *)impl_InterlockedCompareExchange;
void *imp_InterlockedExchange __asm__("__imp___InterlockedExchange@8") = (void *)impl_InterlockedExchange;
void *imp_InterlockedExchangeAdd __asm__("__imp___InterlockedExchangeAdd@8") = (void *)impl_InterlockedExchangeAdd;
void *imp_InterlockedIncrement __asm__("__imp___InterlockedIncrement@4") = (void *)impl_InterlockedIncrement;
void *imp_InterlockedDecrement __asm__("__imp___InterlockedDecrement@4") = (void *)impl_InterlockedDecrement;
