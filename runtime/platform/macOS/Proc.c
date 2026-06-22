/*===-- runtime/macOS/Proc.c - Process control via _exit --------------===*/

#include "../Runtime.h"
#include "LibSystem.h"

FLY_NORETURN void proc_exit(i32 code)
{
    _exit(code);
    FLY_UNREACHABLE();
}

/* libSystem process-control symbols (no <unistd.h>/<sys/wait.h> needed). */
extern int    fork(void);
extern int    execve(const char *path, char *const argv[], char *const envp[]);
extern int    waitpid(int pid, int *status, int options);
extern char **environ;

i32 proc_exec(const char *path, char *const argv[])
{
    int pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        execve(path, argv, environ);   /* returns only on failure */
        _exit(127);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
        return -1;
    if ((status & 0x7f) == 0)               /* WIFEXITED  */
        return (i32)((status >> 8) & 0xff); /* WEXITSTATUS */
    return (i32)(128 + (status & 0x7f));    /* killed by signal: 128 + signo */
}
