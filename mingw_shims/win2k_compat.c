// win2k_compat.c - supply the Windows API surface that the statically
// linked GCC runtime (libgcc gthr-win32) and VS2005-era CRT calls need
// but Windows 2000 does not export:
//
//   kernel32 (Vista):  InitializeConditionVariable,
//                      SleepConditionVariableCS, WakeConditionVariable,
//                      WakeAllConditionVariable
//   kernel32 (XP):     GetThreadId
//   msvcrt (_s family): wcscpy_s, wcscat_s, wcsncpy_s, wcsncat_s,
//                      _snwprintf_s, _vsnwprintf_s,
//                      _wmakepath_s, _wsplitpath_s
//
// Same technique as interlocked_shim.c: local implementations bound to
// the exact linker symbols, plus the __imp_ pointer cells that
// dllimport-style callers use (both underscore spellings are defined;
// an unused cell costs nothing).
//
// The condition variable is the classic semaphore + done-event design
// (Schmidt/Pyarali CV_9) adapted to SleepConditionVariableCS, where
// the caller's lock is a CRITICAL_SECTION. State is allocated lazily
// on first use, so a statically zero-initialised CONDITION_VARIABLE
// (CONDITION_VARIABLE_INIT) works without InitializeConditionVariable,
// matching the Vista contract. Known limitation, shared with the
// classic design: a signal that races with a waiter's timeout can
// leave the semaphore count off by one.
#include <windows.h>
#include <stddef.h>
#include <stdarg.h>

size_t wcslen(const wchar_t *s);
wchar_t *wcscpy(wchar_t *d, const wchar_t *s);
void *memcpy(void *d, const void *s, size_t n);
int _vsnwprintf(wchar_t *buf, size_t count, const wchar_t *fmt, va_list ap);

/* ---------------- GetThreadId (via ntdll) ---------------- */

typedef struct {
    long ExitStatus;
    void *TebBaseAddress;
    void *UniqueProcess;
    void *UniqueThread;
    unsigned long AffinityMask;
    long Priority;
    long BasePriority;
} shim_tbi_t;
typedef long (__attribute__((stdcall)) *shim_ntqit_t)(void *h, int cls, void *info, unsigned long len, unsigned long *retlen);

DWORD __attribute__((stdcall)) impl_GetThreadId(HANDLE h)
__asm__("_GetThreadId@4");
DWORD __attribute__((stdcall)) impl_GetThreadId(HANDLE h)
{
    static shim_ntqit_t fn;
    if (!fn) {
        HMODULE n = GetModuleHandleW(L"ntdll.dll");
        if (n)
            fn = (shim_ntqit_t)(void *)GetProcAddress(n, "NtQueryInformationThread");
        if (!fn)
            return 0;
    }
    shim_tbi_t tbi;
    if (fn(h, 0, &tbi, sizeof tbi, NULL) != 0)
        return 0;
    return (DWORD)(DWORD_PTR)tbi.UniqueThread;
}
void *imp_GetThreadId_a __asm__("__imp___GetThreadId@4") = (void *)impl_GetThreadId;
void *imp_GetThreadId_b __asm__("__imp__GetThreadId@4") = (void *)impl_GetThreadId;

/* ---------------- condition variables ---------------- */

typedef struct {
    CRITICAL_SECTION lock;
    HANDLE sema;
    HANDLE done;
    unsigned waiters;
    int was_broadcast;
} shim_cv_t;

static shim_cv_t *shim_cv_get(void **cv)
{
    shim_cv_t *s = (shim_cv_t *)*cv;
    if (s)
        return s;
    s = (shim_cv_t *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof *s);
    InitializeCriticalSection(&s->lock);
    s->sema = CreateSemaphoreW(NULL, 0, 0x7fffffff, NULL);
    s->done = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (__sync_val_compare_and_swap(cv, (void *)0, (void *)s) != (void *)0) {
        CloseHandle(s->sema);
        CloseHandle(s->done);
        DeleteCriticalSection(&s->lock);
        HeapFree(GetProcessHeap(), 0, s);
        return (shim_cv_t *)*cv;
    }
    return s;
}

void __attribute__((stdcall)) impl_InitializeConditionVariable(void **cv)
__asm__("_InitializeConditionVariable@4");
void __attribute__((stdcall)) impl_InitializeConditionVariable(void **cv)
{
    *cv = (void *)0;
}
void *imp_InitCV_a __asm__("__imp___InitializeConditionVariable@4") = (void *)impl_InitializeConditionVariable;
void *imp_InitCV_b __asm__("__imp__InitializeConditionVariable@4") = (void *)impl_InitializeConditionVariable;

int __attribute__((stdcall)) impl_SleepConditionVariableCS(void **cv, CRITICAL_SECTION *cs, DWORD ms)
__asm__("_SleepConditionVariableCS@12");
int __attribute__((stdcall)) impl_SleepConditionVariableCS(void **cv, CRITICAL_SECTION *cs, DWORD ms)
{
    shim_cv_t *s = shim_cv_get(cv);
    DWORD r;
    int last;
    EnterCriticalSection(&s->lock);
    s->waiters++;
    LeaveCriticalSection(&s->lock);
    LeaveCriticalSection(cs);
    r = WaitForSingleObject(s->sema, ms);
    EnterCriticalSection(&s->lock);
    s->waiters--;
    last = s->was_broadcast && s->waiters == 0;
    LeaveCriticalSection(&s->lock);
    if (last)
        SetEvent(s->done);
    EnterCriticalSection(cs);
    if (r != WAIT_OBJECT_0) {
        SetLastError(1460); /* ERROR_TIMEOUT */
        return 0;
    }
    return 1;
}
void *imp_SleepCV_a __asm__("__imp___SleepConditionVariableCS@12") = (void *)impl_SleepConditionVariableCS;
void *imp_SleepCV_b __asm__("__imp__SleepConditionVariableCS@12") = (void *)impl_SleepConditionVariableCS;

void __attribute__((stdcall)) impl_WakeConditionVariable(void **cv)
__asm__("_WakeConditionVariable@4");
void __attribute__((stdcall)) impl_WakeConditionVariable(void **cv)
{
    shim_cv_t *s = shim_cv_get(cv);
    int have;
    EnterCriticalSection(&s->lock);
    have = s->waiters > 0;
    LeaveCriticalSection(&s->lock);
    if (have)
        ReleaseSemaphore(s->sema, 1, NULL);
}
void *imp_WakeCV_a __asm__("__imp___WakeConditionVariable@4") = (void *)impl_WakeConditionVariable;
void *imp_WakeCV_b __asm__("__imp__WakeConditionVariable@4") = (void *)impl_WakeConditionVariable;

void __attribute__((stdcall)) impl_WakeAllConditionVariable(void **cv)
__asm__("_WakeAllConditionVariable@4");
void __attribute__((stdcall)) impl_WakeAllConditionVariable(void **cv)
{
    shim_cv_t *s = shim_cv_get(cv);
    int have = 0;
    EnterCriticalSection(&s->lock);
    if (s->waiters > 0) {
        s->was_broadcast = 1;
        have = 1;
        ReleaseSemaphore(s->sema, (LONG)s->waiters, NULL);
    }
    LeaveCriticalSection(&s->lock);
    if (have) {
        WaitForSingleObject(s->done, INFINITE);
        s->was_broadcast = 0;
    }
}
void *imp_WakeAllCV_a __asm__("__imp___WakeAllConditionVariable@4") = (void *)impl_WakeAllConditionVariable;
void *imp_WakeAllCV_b __asm__("__imp__WakeAllConditionVariable@4") = (void *)impl_WakeAllConditionVariable;

/* ---------------- msvcrt _s family (cdecl) ---------------- */
/* return codes: 22 = EINVAL, 34 = ERANGE, 80 = STRUNCATE */

int impl_wcscpy_s(wchar_t *dst, size_t size, const wchar_t *src)
__asm__("_wcscpy_s");
int impl_wcscpy_s(wchar_t *dst, size_t size, const wchar_t *src)
{
    if (!dst || size == 0)
        return 22;
    if (!src) {
        dst[0] = 0;
        return 22;
    }
    if (wcslen(src) >= size) {
        dst[0] = 0;
        return 34;
    }
    wcscpy(dst, src);
    return 0;
}
void *imp_wcscpy_s __asm__("__imp__wcscpy_s") = (void *)impl_wcscpy_s;

int impl_wcscat_s(wchar_t *dst, size_t size, const wchar_t *src)
__asm__("_wcscat_s");
int impl_wcscat_s(wchar_t *dst, size_t size, const wchar_t *src)
{
    size_t dlen = 0;
    if (!dst || size == 0)
        return 22;
    while (dlen < size && dst[dlen])
        dlen++;
    if (dlen == size || !src) {
        if (dlen < size)
            dst[0] = 0;
        return 22;
    }
    if (dlen + wcslen(src) >= size) {
        dst[0] = 0;
        return 34;
    }
    wcscpy(dst + dlen, src);
    return 0;
}
void *imp_wcscat_s __asm__("__imp__wcscat_s") = (void *)impl_wcscat_s;

int impl_wcsncpy_s(wchar_t *dst, size_t size, const wchar_t *src, size_t count)
__asm__("_wcsncpy_s");
int impl_wcsncpy_s(wchar_t *dst, size_t size, const wchar_t *src, size_t count)
{
    size_t slen = 0;
    if (!dst || size == 0)
        return 22;
    if (!src) {
        dst[0] = 0;
        return 22;
    }
    if (count == (size_t)-1) {
        slen = wcslen(src);
        if (slen >= size) {
            memcpy(dst, src, (size - 1) * sizeof(wchar_t));
            dst[size - 1] = 0;
            return 80;
        }
        wcscpy(dst, src);
        return 0;
    }
    while (slen < count && src[slen])
        slen++;
    if (slen >= size) {
        dst[0] = 0;
        return 34;
    }
    memcpy(dst, src, slen * sizeof(wchar_t));
    dst[slen] = 0;
    return 0;
}
void *imp_wcsncpy_s __asm__("__imp__wcsncpy_s") = (void *)impl_wcsncpy_s;

int impl_wcsncat_s(wchar_t *dst, size_t size, const wchar_t *src, size_t count)
__asm__("_wcsncat_s");
int impl_wcsncat_s(wchar_t *dst, size_t size, const wchar_t *src, size_t count)
{
    size_t dlen = 0, slen = 0;
    if (!dst || size == 0)
        return 22;
    while (dlen < size && dst[dlen])
        dlen++;
    if (dlen == size || !src)
        return 22;
    if (count == (size_t)-1) {
        slen = wcslen(src);
        if (dlen + slen >= size) {
            size_t room = size - 1 - dlen;
            memcpy(dst + dlen, src, room * sizeof(wchar_t));
            dst[size - 1] = 0;
            return 80;
        }
        wcscpy(dst + dlen, src);
        return 0;
    }
    while (slen < count && src[slen])
        slen++;
    if (dlen + slen >= size) {
        dst[0] = 0;
        return 34;
    }
    memcpy(dst + dlen, src, slen * sizeof(wchar_t));
    dst[dlen + slen] = 0;
    return 0;
}
void *imp_wcsncat_s __asm__("__imp__wcsncat_s") = (void *)impl_wcsncat_s;

int impl_vsnwprintf_s(wchar_t *buf, size_t size, size_t count, const wchar_t *fmt, va_list ap)
__asm__("__vsnwprintf_s");
int impl_vsnwprintf_s(wchar_t *buf, size_t size, size_t count, const wchar_t *fmt, va_list ap)
{
    size_t want;
    int r;
    if (!buf || size == 0 || !fmt)
        return -1;
    want = (count == (size_t)-1) ? size - 1 : count;
    if (want >= size)
        want = size - 1;
    r = _vsnwprintf(buf, want, fmt, ap);
    if (r >= 0) {
        buf[r] = 0;
        return r;
    }
    if (count == (size_t)-1) {
        buf[want] = 0;
        return (int)want;
    }
    buf[0] = 0;
    return -1;
}
void *imp_vsnwprintf_s __asm__("__imp___vsnwprintf_s") = (void *)impl_vsnwprintf_s;

int impl_snwprintf_s(wchar_t *buf, size_t size, size_t count, const wchar_t *fmt, ...)
__asm__("__snwprintf_s");
int impl_snwprintf_s(wchar_t *buf, size_t size, size_t count, const wchar_t *fmt, ...)
{
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = impl_vsnwprintf_s(buf, size, count, fmt, ap);
    va_end(ap);
    return r;
}
void *imp_snwprintf_s __asm__("__imp___snwprintf_s") = (void *)impl_snwprintf_s;

int impl_wmakepath_s(wchar_t *path, size_t size, const wchar_t *drive, const wchar_t *dir, const wchar_t *fname, const wchar_t *ext)
__asm__("__wmakepath_s");
int impl_wmakepath_s(wchar_t *path, size_t size, const wchar_t *drive, const wchar_t *dir, const wchar_t *fname, const wchar_t *ext)
{
    wchar_t tmp[260];
    size_t n = 0, i;
    const wchar_t *parts[4];
    if (!path || size == 0)
        return 22;
    parts[0] = drive;
    parts[1] = dir;
    parts[2] = fname;
    parts[3] = ext;
    for (i = 0; i < 4; i++) {
        const wchar_t *p = parts[i];
        size_t len;
        if (!p)
            continue;
        if (i == 1 && n > 0 && tmp[n - 1] != L'\\' && tmp[n - 1] != L'/' && p[0] != L'\\' && p[0] != L'/' && tmp[n - 1] != L':')
            tmp[n++] = L'\\';
        if (i == 2 && n > 0 && tmp[n - 1] != L'\\' && tmp[n - 1] != L'/' && p[0])
            tmp[n++] = L'\\';
        if (i == 3 && p[0] && p[0] != L'.')
            tmp[n++] = L'.';
        len = wcslen(p);
        if (n + len >= sizeof tmp / sizeof tmp[0]) {
            path[0] = 0;
            return 34;
        }
        memcpy(tmp + n, p, len * sizeof(wchar_t));
        n += len;
    }
    tmp[n] = 0;
    if (n >= size) {
        path[0] = 0;
        return 34;
    }
    wcscpy(path, tmp);
    return 0;
}
void *imp_wmakepath_s __asm__("__imp___wmakepath_s") = (void *)impl_wmakepath_s;

static int shim_copy_part(wchar_t *out, size_t outsize, const wchar_t *begin, size_t len)
{
    if (!out)
        return 0;
    if (outsize == 0)
        return 34;
    if (len >= outsize) {
        out[0] = 0;
        return 34;
    }
    memcpy(out, begin, len * sizeof(wchar_t));
    out[len] = 0;
    return 0;
}

int impl_wsplitpath_s(const wchar_t *path, wchar_t *drive, size_t dsize, wchar_t *dir, size_t dirsize, wchar_t *fname, size_t fsize, wchar_t *ext, size_t esize)
__asm__("__wsplitpath_s");
int impl_wsplitpath_s(const wchar_t *path, wchar_t *drive, size_t dsize, wchar_t *dir, size_t dirsize, wchar_t *fname, size_t fsize, wchar_t *ext, size_t esize)
{
    const wchar_t *p, *lastslash, *dot, *comp;
    int rc = 0;
    if (!path)
        return 22;
    p = path;
    if (p[0] && p[1] == L':') {
        rc |= shim_copy_part(drive, dsize, p, 2);
        p += 2;
    } else if (drive && dsize)
        drive[0] = 0;
    lastslash = (const wchar_t *)0;
    {
        const wchar_t *q;
        for (q = p; *q; q++)
            if (*q == L'\\' || *q == L'/')
                lastslash = q;
    }
    if (lastslash) {
        rc |= shim_copy_part(dir, dirsize, p, (size_t)(lastslash - p) + 1);
        comp = lastslash + 1;
    } else {
        if (dir && dirsize)
            dir[0] = 0;
        comp = p;
    }
    dot = (const wchar_t *)0;
    {
        const wchar_t *q;
        for (q = comp; *q; q++)
            if (*q == L'.')
                dot = q;
    }
    if (dot) {
        rc |= shim_copy_part(fname, fsize, comp, (size_t)(dot - comp));
        rc |= shim_copy_part(ext, esize, dot, wcslen(dot));
    } else {
        rc |= shim_copy_part(fname, fsize, comp, wcslen(comp));
        if (ext && esize)
            ext[0] = 0;
    }
    if (rc) {
        if (drive && dsize)
            drive[0] = 0;
        if (dir && dirsize)
            dir[0] = 0;
        if (fname && fsize)
            fname[0] = 0;
        if (ext && esize)
            ext[0] = 0;
        return 34;
    }
    return 0;
}
void *imp_wsplitpath_s __asm__("__imp___wsplitpath_s") = (void *)impl_wsplitpath_s;
