/*===-- runtime/Linux/Env.c - Environment primitives via Linux syscalls ---===*/

#include "../Runtime.h"
#include "Syscall.h"

/* libc symbols used for environment access (no <stdlib.h> needed) */
extern char **environ;
extern char  *getenv(const char *name);
extern int    setenv(const char *name, const char *value, int overwrite);
extern int    unsetenv(const char *name);
extern usize  strlen(const char *s);
extern char  *strcpy(char *dst, const char *src);
extern char  *strchr(const char *s, int c);

/* Linux uname struct (x86-64) */
typedef struct {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
} linux_utsname_t;

/* Argv storage — populated by env_init() called from the fly runtime entry */
static int    g_argc = 0;
static char **g_argv = (char **)0;

void env_init(int argc, char **argv)
{
    g_argc = argc;
    g_argv = argv;
}

i32 env_getcwd(char *buf, usize cap)
{
    long r = __syscall2(SYS_getcwd, (long)buf, (long)cap);
    if (r <= 0) return -1;
    /* r includes the null terminator; return path length without it */
    return (i32)(r - 1);
}

i32 env_chdir(const char *path)
{
    long r = __syscall1(SYS_chdir, (long)path);
    return (r == 0) ? 0 : -1;
}

i32 env_get(const char *key, char *buf, usize size)
{
    char *val = getenv(key);
    if (!val) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = strlen(val);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = val[i];
    buf[i] = '\0';
    return (i32)len;
}

i32 env_set(const char *key, const char *val)
{
    return setenv(key, val, 1) == 0 ? 0 : -1;
}

i32 env_delete(const char *key)
{
    return unsetenv(key) == 0 ? 0 : -1;
}

i32 env_all_count(void)
{
    i32 n = 0;
    if (!environ) return 0;
    while (environ[n]) n++;
    return n;
}

void env_all_get(i32 idx, char *key_buf, usize key_size,
                 char *val_buf, usize val_size)
{
    const char *entry = environ[idx];
    if (!entry) {
        if (key_size > 0) key_buf[0] = '\0';
        if (val_size > 0) val_buf[0] = '\0';
        return;
    }
    const char *eq = strchr(entry, '=');
    usize klen = eq ? (usize)(eq - entry) : strlen(entry);
    if (klen >= key_size) klen = key_size - 1;
    usize i;
    for (i = 0; i < klen; i++) key_buf[i] = entry[i];
    key_buf[i] = '\0';

    const char *vstart = eq ? eq + 1 : "";
    usize vlen = strlen(vstart);
    if (vlen >= val_size) vlen = val_size - 1;
    for (i = 0; i < vlen; i++) val_buf[i] = vstart[i];
    val_buf[i] = '\0';
}

i32 env_hostname(char *buf, usize size)
{
    linux_utsname_t u;
    long r = __syscall1(SYS_uname, (long)&u);
    if (r < 0) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = strlen(u.nodename);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = u.nodename[i];
    buf[i] = '\0';
    return (i32)len;
}

/* env_arch / env_kernel / env_kernelversion — host details from a uname(2) syscall
 * (machine / release / version), same copy-into-buf contract as env_hostname. */
i32 env_arch(char *buf, usize size)
{
    linux_utsname_t u;
    long r = __syscall1(SYS_uname, (long)&u);
    if (r < 0) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = strlen(u.machine);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = u.machine[i];
    buf[i] = '\0';
    return (i32)len;
}

i32 env_kernel(char *buf, usize size)
{
    linux_utsname_t u;
    long r = __syscall1(SYS_uname, (long)&u);
    if (r < 0) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = strlen(u.release);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = u.release[i];
    buf[i] = '\0';
    return (i32)len;
}

i32 env_kernelversion(char *buf, usize size)
{
    linux_utsname_t u;
    long r = __syscall1(SYS_uname, (long)&u);
    if (r < 0) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = strlen(u.version);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = u.version[i];
    buf[i] = '\0';
    return (i32)len;
}

/* env_exe_path — absolute path of the running executable via readlink(2) on
 * /proc/self/exe. Independent of argv[0]/$PATH; the Fly equivalent of
 * std::env::current_exe (cf. getMainExecutable in the C++ driver). readlink does
 * not NUL-terminate, so reserve a byte and add the terminator ourselves. */
i32 env_exe_path(char *buf, usize size)
{
    if (size == 0) return -1;
    long r = __syscall3(SYS_readlink, (long)"/proc/self/exe",
                        (long)buf, (long)(size - 1u));
    if (r < 0) { buf[0] = '\0'; return -1; }
    buf[(usize)r] = '\0';
    return (i32)r;
}

i32 env_args_count(void)
{
    return g_argc;
}

i32 env_args_get(i32 idx, char *buf, usize size)
{
    if (idx < 0 || idx >= g_argc || !g_argv) {
        if (size > 0) buf[0] = '\0';
        return -1;
    }
    const char *arg = g_argv[idx];
    usize len = strlen(arg);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = arg[i];
    buf[i] = '\0';
    return (i32)len;
}

/* ── Fly string-array slot helpers ─────────────────────────────────────────── */
/* Fly string struct layout in memory: { i64 ptr, i32 size, i32 _pad } = 16 B */
typedef struct { i64 p; i32 s; i32 _pad; } FlyStrSlot;

void str_slot_set(char *arr, i32 idx, char *ptr, i32 size)
{
    FlyStrSlot *fs = (FlyStrSlot *)(arr + (usize)idx * 16u);
    fs->p   = (i64)(usize)ptr;
    fs->s   = size;
    fs->_pad = 0;
}

void str_slot_get(char *arr, i32 idx, i64 *ptr_out, i32 *size_out)
{
    FlyStrSlot *fs = (FlyStrSlot *)(arr + (usize)idx * 16u);
    *ptr_out  = fs->p;
    *size_out = fs->s;
}

i64 str_slot_ptr(char *arr, i32 idx)
{
    FlyStrSlot *fs = (FlyStrSlot *)(arr + (usize)idx * 16u);
    return fs->p;
}

i32 str_slot_size(char *arr, i32 idx)
{
    FlyStrSlot *fs = (FlyStrSlot *)(arr + (usize)idx * 16u);
    return fs->s;
}


void env_args_fill(char *arr, i32 count)
{
    i32 i;
    for (i = 0; i < count && i < g_argc && g_argv; i++) {
        const char *arg = g_argv[i];
        usize len = strlen(arg);
        char *buf = (char *)mem_alloc(len + 1u);
        if (!buf) { str_slot_set(arr, i, (char *)0, 0); continue; }
        usize j;
        for (j = 0; j < len; j++) buf[j] = arg[j];
        buf[j] = '\0';
        str_slot_set(arr, i, buf, (i32)len);
    }
}

void env_all_fill(char *arr, i32 count)
{
    if (!environ) return;
    i32 i;
    for (i = 0; i < count; i++) {
        const char *entry = environ[i];
        if (!entry) break;
        usize len = strlen(entry);
        char *buf = (char *)mem_alloc(len + 1u);
        if (!buf) { str_slot_set(arr, i, (char *)0, 0); continue; }
        usize j;
        for (j = 0; j < len; j++) buf[j] = entry[j];
        buf[j] = '\0';
        str_slot_set(arr, i, buf, (i32)len);
    }
}

i32 env_osname(char *buf, usize size)
{
    const char *name = "linux";
    usize len = 5u;
    if (len >= size) len = size - 1u;
    usize i;
    for (i = 0; i < len; i++) buf[i] = name[i];
    buf[i] = '\0';
    return (i32)len;
}
