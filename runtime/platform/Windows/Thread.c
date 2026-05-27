/*===-- runtime/Windows/Thread.c - Threads via CreateThread -----------===
 *
 * CreateThread requires DWORD WINAPI (LPVOID); a trampoline bridges it to
 * the fly void (*)(void *) signature.  The ThreadCtx is freed by the child
 * before calling the user function.
 *===----------------------------------------------------------------------===*/

#include "../Runtime.h"
#include "Win32.h"

#ifdef _MSC_VER
#  include <intrin.h>
#  pragma intrinsic(_InterlockedOr, _InterlockedExchange, \
                    _InterlockedCompareExchange, _InterlockedExchangeAdd)
#endif

typedef struct { void (*fn)(void *); void *arg; } ThreadCtx;

static DWORD WINAPI trampoline(LPVOID param)
{
    ThreadCtx *ctx = (ThreadCtx *)param;
    void (*fn)(void *) = ctx->fn;
    void *arg          = ctx->arg;
    mem_free(ctx, sizeof(ThreadCtx));
    fn(arg);
    return 0;
}

tid thread_spawn(void (*fn)(void *), void *arg, usize stack_size)
{
    ThreadCtx *ctx = (ThreadCtx *)mem_alloc(sizeof(ThreadCtx));
    if (!ctx) return -12;
    ctx->fn = fn;
    ctx->arg = arg;

    DWORD thread_id = 0;
    HANDLE h = CreateThread(WIN32_NULL, (SIZE_T)stack_size,
                            trampoline, (LPVOID)ctx, 0, &thread_id);
    if (!h) { mem_free(ctx, sizeof(ThreadCtx)); return -1; }
    CloseHandle(h);
    return (tid)thread_id;
}

i32 futex_wait(i32 *addr, i32 expected)
{
    BOOL ok = WaitOnAddress((volatile void *)addr, (LPVOID)&expected,
                            sizeof(i32), WIN32_INFINITE);
    return ok ? 0 : -1;
}

i32 futex_wake(i32 *addr, i32 count)
{
    if (count == 1) { WakeByAddressSingle((LPVOID)addr); return 1; }
    WakeByAddressAll((LPVOID)addr);
    return count;
}

i32 atomic_load_i32(i32 *addr) {
#ifdef _MSC_VER
    return (i32)_InterlockedOr((volatile long *)addr, 0);
#else
    return __atomic_load_n(addr, __ATOMIC_ACQUIRE);
#endif
}
void atomic_store_i32(i32 *addr, i32 val) {
#ifdef _MSC_VER
    _InterlockedExchange((volatile long *)addr, (long)val);
#else
    __atomic_store_n(addr, val, __ATOMIC_RELEASE);
#endif
}
i32 atomic_cas_i32(i32 *addr, i32 expected, i32 desired) {
#ifdef _MSC_VER
    return (i32)_InterlockedCompareExchange((volatile long *)addr, (long)desired, (long)expected);
#else
    __atomic_compare_exchange_n(addr, &expected, desired, 0,
                                __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    return expected;
#endif
}
i32 atomic_fetch_add_i32(i32 *addr, i32 delta) {
#ifdef _MSC_VER
    return (i32)_InterlockedExchangeAdd((volatile long *)addr, (long)delta);
#else
    return __atomic_fetch_add(addr, delta, __ATOMIC_ACQ_REL);
#endif
}
