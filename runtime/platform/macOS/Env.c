/*===-- runtime/macOS/Env.c - Environment primitives via libSystem -------===*/

#include "../Runtime.h"
#include "LibSystem.h"

static int g_argc = 0;
static char **g_argv = (char **)0;

void env_init(int argc, char **argv)
{
    g_argc = argc;
    g_argv = argv;
}

static i32 cstr_len(const char *s) { i32 n = 0; while (s[n]) n++; return n; }

i32 env_getcwd(char *buf, usize cap)
{
    char *p = getcwd(buf, (size_rt)cap);
    if (!p) return -1;
    return cstr_len(buf);
}

i32 env_chdir(const char *path)
{
    return (chdir(path) == 0) ? 0 : -1;
}

i32 env_get(const char *key, char *buf, usize size)
{
    char *val = getenv(key);
    if (!val) { if (size > 0) buf[0] = '\0'; return -1; }
    usize len = (usize)strlen(val);
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
    usize klen = eq ? (usize)(eq - entry) : (usize)strlen(entry);
    if (klen >= key_size) klen = key_size - 1;
    usize i;
    for (i = 0; i < klen; i++) key_buf[i] = entry[i];
    key_buf[i] = '\0';

    const char *vstart = eq ? eq + 1 : "";
    usize vlen = (usize)strlen(vstart);
    if (vlen >= val_size) vlen = val_size - 1;
    for (i = 0; i < vlen; i++) val_buf[i] = vstart[i];
    val_buf[i] = '\0';
}

i32 env_hostname(char *buf, usize size)
{
    if (gethostname(buf, (size_rt)size) != 0) { if (size > 0) buf[0] = '\0'; return -1; }
    return cstr_len(buf);
}

/* Host details from uname(3). macOS _SYS_NAMELEN == 256 for every field. */
struct fly_utsname { char sysname[256]; char nodename[256]; char release[256];
                     char version[256]; char machine[256]; };
extern int uname(struct fly_utsname *);

static i32 fly_copy_field(const char *field, char *buf, usize size)
{
    usize len = (usize)strlen(field);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = field[i];
    buf[i] = '\0';
    return (i32)len;
}

i32 env_arch(char *buf, usize size)
{
    struct fly_utsname u;
    if (uname(&u) != 0) { if (size > 0) buf[0] = '\0'; return -1; }
    return fly_copy_field(u.machine, buf, size);
}

i32 env_kernel(char *buf, usize size)
{
    struct fly_utsname u;
    if (uname(&u) != 0) { if (size > 0) buf[0] = '\0'; return -1; }
    return fly_copy_field(u.release, buf, size);
}

i32 env_kernelversion(char *buf, usize size)
{
    struct fly_utsname u;
    if (uname(&u) != 0) { if (size > 0) buf[0] = '\0'; return -1; }
    return fly_copy_field(u.version, buf, size);
}

/* env_exe_path — running executable path via _NSGetExecutablePath. Independent
 * of argv[0]; the Fly equivalent of std::env::current_exe. The function NUL-
 * terminates buf and returns non-zero if it is too small. */
extern int _NSGetExecutablePath(char *buf, unsigned int *bufsize);

i32 env_exe_path(char *buf, usize size)
{
    if (size == 0) return -1;
    unsigned int bufsize = (unsigned int)size;
    if (_NSGetExecutablePath(buf, &bufsize) != 0) { buf[0] = '\0'; return -1; }
    return (i32)strlen(buf);
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
    usize len = (usize)strlen(arg);
    if (len >= size) len = size - 1;
    usize i;
    for (i = 0; i < len; i++) buf[i] = arg[i];
    buf[i] = '\0';
    return (i32)len;
}

/* ── Fly string-array slot helpers ─────────────────────────────────────────── */
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
        usize len = (usize)strlen(arg);
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
        usize len = (usize)strlen(entry);
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
    const char *name = "macos";
    usize len = 5u;
    if (len >= size) len = size - 1u;
    usize i;
    for (i = 0; i < len; i++) buf[i] = name[i];
    buf[i] = '\0';
    return (i32)len;
}
