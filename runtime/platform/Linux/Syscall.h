/*===-- src/Runtime/Linux/Syscall.h - Linux x86-64 raw syscall helpers ----===
 *
 * Private header — not installed, used only by src/Runtime/Linux/ sources.
 *
 * Linux x86-64 syscall ABI:
 *   syscall number  → rax
 *   arg 1           → rdi
 *   arg 2           → rsi
 *   arg 3           → rdx
 *   arg 4           → r10   (NOT rcx — rcx is clobbered by syscall)
 *   arg 5           → r8
 *   arg 6           → r9
 *   return value    → rax   (negative errno on error)
 *   clobbered       → rcx, r11
 *
 * Reference: linux/arch/x86/entry/entry_64.S
 *===----------------------------------------------------------------------===*/

#ifndef FLY_RUNTIME_LINUX_SYSCALL_H
#define FLY_RUNTIME_LINUX_SYSCALL_H

/* ── Syscall number table (x86-64) ─────────────────────────────────────── */

#define SYS_read          0
#define SYS_write         1
#define SYS_open          2
#define SYS_close         3
#define SYS_stat          4
#define SYS_lstat         6
#define SYS_lseek         8
#define SYS_mmap          9
#define SYS_munmap        11
#define SYS_nanosleep     35
#define SYS_clone         56
#define SYS_exit          60
#define SYS_uname         63
#define SYS_fsync         74
#define SYS_truncate      76
#define SYS_getcwd        79
#define SYS_chdir         80
#define SYS_rename        82
#define SYS_mkdir         83
#define SYS_rmdir         84
#define SYS_symlink       88
#define SYS_readlink      89
#define SYS_chmod         90
#define SYS_unlink        87
#define SYS_clock_gettime 228
#define SYS_futex         202
#define SYS_exit_group    231
#define SYS_getdents64    217
#define SYS_fork          57
#define SYS_execve        59
#define SYS_wait4         61

/* ── Inline helpers ─────────────────────────────────────────────────────── */

static __inline__ long __syscall1(long n, long a1)
{
    long ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"(n), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static __inline__ long __syscall2(long n, long a1, long a2)
{
    long ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"(n), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static __inline__ long __syscall3(long n, long a1, long a2, long a3)
{
    long ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

/* 4-arg: r10 is used for arg4 because rcx is clobbered by the syscall
 * instruction itself and cannot be used as an input register. */
static __inline__ long __syscall4(long n, long a1, long a2, long a3, long a4)
{
    long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

/* 6-arg variant used by mmap and clone. */
static __inline__ long __syscall6(long n,
                                   long a1, long a2, long a3,
                                   long a4, long a5, long a6)
{
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8  __asm__("r8")  = a5;
    register long r9  __asm__("r9")  = a6;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

#endif /* FLY_RUNTIME_LINUX_SYSCALL_H */
