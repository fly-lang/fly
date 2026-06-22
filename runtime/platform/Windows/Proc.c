/*===-- runtime/Windows/Proc.c - Process control via ExitProcess ------===*/

#include "../Runtime.h"
#include "Win32.h"

FLY_NORETURN void proc_exit(i32 code)
{
    ExitProcess((DWORD)(unsigned int)code);
    FLY_UNREACHABLE();
}

/* Append the NUL-terminated 's' to buf at *pos (capacity 'cap'), advancing *pos.
 * Silently truncates if the buffer would overflow (leaving room for a final \0). */
static void cmd_append(char *buf, usize *pos, usize cap, const char *s)
{
    usize p = *pos;
    while (*s && p + 1 < cap)
        buf[p++] = *s++;
    *pos = p;
}

i32 proc_exec(const char *path, char *const argv[])
{
    /* Build a single command-line string: "arg0" arg1 arg2 ...
     * The first token is double-quoted so a path with spaces stays intact;
     * remaining args are appended space-separated (no per-arg quoting — callers
     * that need it must pre-quote, matching the simple POSIX execve contract). */
    char cmd[8192];
    usize pos = 0;
    cmd_append(cmd, &pos, sizeof(cmd), "\"");
    cmd_append(cmd, &pos, sizeof(cmd), path);
    cmd_append(cmd, &pos, sizeof(cmd), "\"");
    if (argv) {
        for (i32 i = 1; argv[i] != 0; i++) {
            cmd_append(cmd, &pos, sizeof(cmd), " ");
            cmd_append(cmd, &pos, sizeof(cmd), argv[i]);
        }
    }
    cmd[pos] = 0;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    /* zero-initialise both structs */
    for (usize k = 0; k < sizeof(si); k++) ((char *)&si)[k] = 0;
    for (usize k = 0; k < sizeof(pi); k++) ((char *)&pi)[k] = 0;
    si.cb = (DWORD)sizeof(si);

    if (!CreateProcessA(0, cmd, 0, 0, 1 /*TRUE*/, 0, 0, 0, &si, &pi))
        return -1;

    WaitForSingleObject(pi.hProcess, WIN32_INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (i32)code;
}
