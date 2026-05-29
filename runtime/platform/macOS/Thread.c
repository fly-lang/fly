/*===-- runtime/macOS/Thread.c - Threads via pthread/__ulock ----------===
 *
 * pthread_create requires void *(*)(void *); a trampoline bridges it to
 * the fly void (*)(void *) signature.
 *
 * __ulock_wait / __ulock_wake are private XNU syscalls from libSystem.B
 * that provide compare-and-wait semantics equivalent to Linux FUTEX_WAIT/WAKE.
 *===----------------------------------------------------------------------===*/

#include "../Runtime.h"
#include "LibSystem.h"

typedef struct { void (*fn)(void *); void *arg; } ThreadCtx;

static void *trampoline(void *param)
{
    ThreadCtx *ctx = (ThreadCtx *)param;
    void (*fn)(void *) = ctx->fn;
    void *arg          = ctx->arg;
    mem_free(ctx, sizeof(ThreadCtx));
    fn(arg);
    return (void *)0;
}

tid thread_spawn(void (*fn)(void *), void *arg, usize stack_size)
{
    ThreadCtx *ctx = (ThreadCtx *)mem_alloc(sizeof(ThreadCtx));
    if (!ctx) return -12;
    ctx->fn = fn;
    ctx->arg = arg;

    pthread_attr_t_rt attr;
    pthread_attr_init(&attr);
    if (stack_size > 0)
        pthread_attr_setstacksize(&attr, (size_rt)stack_size);

    pthread_t_rt t = (void *)0;
    int ret = pthread_create(&t, &attr, trampoline, (void *)ctx);
    pthread_attr_destroy(&attr);

    if (ret != 0) { mem_free(ctx, sizeof(ThreadCtx)); return -1; }

    /* pthread_t is a pointer; its value is unique and > 0 on macOS. */
    return (tid)(u64)t;
}

i32 futex_wait(i32 *addr, i32 expected)
{
    int ret = __ulock_wait(UL_COMPARE_AND_WAIT, (void *)addr,
                           (unsigned long long)(unsigned int)expected, 0u);
    return (ret == 0) ? 0 : -1;
}

i32 futex_wake(i32 *addr, i32 count)
{
    if (count == 1) {
        __ulock_wake(UL_COMPARE_AND_WAIT, (void *)addr, 0ULL);
        return 1;
    }
    __ulock_wake(UL_COMPARE_AND_WAIT | ULF_WAKE_ALL, (void *)addr, 0ULL);
    return count;
}

i32 atomic_load_i32(i32 *addr) {
    return __atomic_load_n(addr, __ATOMIC_ACQUIRE);
}
void atomic_store_i32(i32 *addr, i32 val) {
    __atomic_store_n(addr, val, __ATOMIC_RELEASE);
}
i32 atomic_cas_i32(i32 *addr, i32 expected, i32 desired) {
    __atomic_compare_exchange_n(addr, &expected, desired, 0,
                                __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    return expected;
}
i32 atomic_fetch_add_i32(i32 *addr, i32 delta) {
    return __atomic_fetch_add(addr, delta, __ATOMIC_ACQ_REL);
}
