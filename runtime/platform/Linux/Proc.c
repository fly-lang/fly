/*===-- runtime/Linux/Proc.c - Process control via libc ---------------===*/

#include "../Runtime.h"

/* libc (forward-declared under -nostdinc). */
extern void _exit(int status);
extern int  fork(void);
extern int  execve(const char *path, char *const argv[], char *const envp[]);
extern int  waitpid(int pid, int *status, int options);
extern char **environ;

FLY_NORETURN void proc_exit(i32 code)
{
    _exit((int)code);
    FLY_UNREACHABLE();
}

i32 proc_exec(const char *path, char *const argv[])
{
    int pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        /* Child: replace the image. execve returns only on failure. */
        execve(path, argv, environ);
        _exit(127);
    }
    /* Parent: wait for the child and decode its status. */
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
        return -1;
    if ((status & 0x7f) == 0)               /* WIFEXITED  */
        return (i32)((status >> 8) & 0xff); /* WEXITSTATUS */
    return (i32)(128 + (status & 0x7f));    /* killed by signal: 128 + signo */
}
