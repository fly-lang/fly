/*===-- runtime/Linux/Thread.c - Threads via pthread / futex ----------===
 *
 * Threads use libc pthread_create (a trampoline bridges the fly
 * void (*)(void *) start routine to pthread's void *(*)(void *)).
 * futex has no libc wrapper, so it goes through libc's syscall().
 * Atomics are compiler builtins (inlined for aligned i32 — no libatomic).
 *===----------------------------------------------------------------------===*/

#include "../Runtime.h"

/* libc (forward-declared under -nostdinc). pthread_t is unsigned long on x86-64. */
typedef unsigned long pthread_t_rt;
extern int  pthread_create(pthread_t_rt *thread, const void *attr,
                           void *(*start_routine)(void *), void *arg);
extern int  pthread_detach(pthread_t_rt thread);
extern long syscall(long number, long a1, long a2, long a3, long a4);

#define SYS_futex   202
#define FUTEX_WAIT  0
#define FUTEX_WAKE  1

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
    (void)stack_size; /* pthread default stack */

    ThreadCtx *ctx = (ThreadCtx *)mem_alloc(sizeof(ThreadCtx));
    if (!ctx) return -12;
    ctx->fn = fn;
    ctx->arg = arg;

    pthread_t_rt t = 0;
    int ret = pthread_create(&t, (const void *)0, trampoline, (void *)ctx);
    if (ret != 0) { mem_free(ctx, sizeof(ThreadCtx)); return -1; }

    pthread_detach(t); /* spawn-and-forget: release resources on exit */
    return (tid)t;
}

i32 futex_wait(i32 *addr, i32 expected)
{
    long ret = syscall(SYS_futex, (long)addr, (long)FUTEX_WAIT, (long)expected, 0L);
    return (ret == 0) ? 0 : -1;
}

i32 futex_wake(i32 *addr, i32 count)
{
    long ret = syscall(SYS_futex, (long)addr, (long)FUTEX_WAKE, (long)count, 0L);
    return (i32)ret;
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
