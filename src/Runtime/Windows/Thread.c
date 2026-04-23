/*===-- src/Runtime/Windows/Thread.c - Threads via CreateThread -----------===
 *
 * CreateThread requires DWORD WINAPI (LPVOID); a trampoline bridges it to
 * the fly void (*)(void *) signature.  The ThreadCtx is freed by the child
 * before calling the user function.
 *===----------------------------------------------------------------------===*/

#include "Runtime/Runtime.h"
#include "Win32.h"

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
    BOOL ok = WaitOnAddress((volatile LPVOID)addr, (LPVOID)&expected,
                            sizeof(i32), WIN32_INFINITE);
    return ok ? 0 : -1;
}

i32 futex_wake(i32 *addr, i32 count)
{
    if (count == 1) { WakeByAddressSingle((LPVOID)addr); return 1; }
    WakeByAddressAll((LPVOID)addr);
    return count;
}
