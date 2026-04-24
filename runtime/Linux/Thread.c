/*===-- runtime/Linux/Thread.c - Threads via clone/futex --------------===
 *
 * clone ABI (x86-64 kernel):
 *   rax=56 rdi=flags rsi=child_stack rdx=parent_tidptr r10=child_tidptr r8=tls
 *   Parent gets TID (rax>0), child gets 0.
 *
 * Stack layout before clone (sp points to top of child stack, grows down):
 *   [sp+0x00] = fn
 *   [sp+0x08] = arg
 *===----------------------------------------------------------------------===*/

#include "../Runtime.h"
#include "Syscall.h"

#define CLONE_VM        0x00000100
#define CLONE_FS        0x00000200
#define CLONE_FILES     0x00000400
#define CLONE_SIGHAND   0x00000800
#define CLONE_SYSVSEM   0x00040000
#define THREAD_FLAGS    (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM | 17)

#define FUTEX_WAIT  0
#define FUTEX_WAKE  1

tid thread_spawn(void (*fn)(void *), void *arg, usize stack_size)
{
    void *stack = mem_alloc(stack_size);
    if (!stack)
        return -12;

    char *sp = (char *)stack + stack_size;
    sp = (char *)((u64)sp & ~(u64)0xf);
    sp -= sizeof(void *); *(void **)sp = arg;
    sp -= sizeof(void *); *(void **)sp = (void *)fn;

    long t;
    register long r10 __asm__("r10") = 0;
    register long r8  __asm__("r8")  = 0;

    __asm__ __volatile__(
        "syscall\n\t"
        "test %%rax, %%rax\n\t"
        "jnz 1f\n\t"
        "pop %%rax\n\t"
        "pop %%rdi\n\t"
        "call *%%rax\n\t"
        "mov $60, %%rax\n\t"
        "xor %%rdi, %%rdi\n\t"
        "syscall\n\t"
        "1:\n\t"
        : "=a"(t)
        : "0"((long)SYS_clone), "D"((long)THREAD_FLAGS), "S"(sp),
          "d"(0L), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return (tid)t;
}

i32 futex_wait(i32 *addr, i32 expected)
{
    long ret = __syscall4(SYS_futex, (long)addr, (long)FUTEX_WAIT,
                          (long)expected, 0L);
    return (ret == 0) ? 0 : -1;
}

i32 futex_wake(i32 *addr, i32 count)
{
    long ret = __syscall3(SYS_futex, (long)addr, (long)FUTEX_WAKE, (long)count);
    return (i32)ret;
}
