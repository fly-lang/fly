/*===-- runtime/Linux/Proc.c - Process control via exit_group ---------===*/

#include "../Runtime.h"
#include "Syscall.h"

FLY_NORETURN void proc_exit(i32 code)
{
    __syscall1(SYS_exit_group, (long)code);
    FLY_UNREACHABLE();
}

/* Inherited environment (libc symbol, no <unistd.h> needed). */
extern char **environ;

i32 proc_exec(const char *path, char *const argv[])
{
    long pid = __syscall1(SYS_fork, 0);   /* extra arg ignored by the kernel */
    if (pid < 0)
        return -1;
    if (pid == 0) {
        /* Child: replace the image. execve returns only on failure. */
        __syscall3(SYS_execve, (long)path, (long)argv, (long)environ);
        proc_exit(127);
    }
    /* Parent: wait for the child and decode its status. */
    int status = 0;
    long r = __syscall4(SYS_wait4, pid, (long)&status, 0, 0);
    if (r < 0)
        return -1;
    if ((status & 0x7f) == 0)               /* WIFEXITED  */
        return (i32)((status >> 8) & 0xff); /* WEXITSTATUS */
    return (i32)(128 + (status & 0x7f));    /* killed by signal: 128 + signo */
}
