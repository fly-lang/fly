/*===-- runtime/Windows/Env.c - Environment primitives via Win32 ---------===*/

#include "../Runtime.h"
#include "Win32.h"

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
    DWORD r = GetCurrentDirectoryA((DWORD)cap, buf);
    return (r > 0 && r < (DWORD)cap) ? (i32)r : -1;
}

i32 env_chdir(const char *path)
{
    return SetCurrentDirectoryA(path) ? 0 : -1;
}

i32 env_get(const char *key, char *buf, usize size)
{
    DWORD r = GetEnvironmentVariableA(key, buf, (DWORD)size);
    return (r > 0 && r < (DWORD)size) ? (i32)r : -1;
}

i32 env_set(const char *key, const char *val)
{
    return SetEnvironmentVariableA(key, val) ? 0 : -1;
}

i32 env_delete(const char *key)
{
    return SetEnvironmentVariableA(key, WIN32_NULL) ? 0 : -1;
}

i32 env_all_count(void)
{
    char *env = GetEnvironmentStringsA();
    if (!env) return 0;
    i32 n = 0;
    const char *p = env;
    while (*p) { n++; while (*p) p++; p++; }
    FreeEnvironmentStringsA(env);
    return n;
}

void env_all_get(i32 idx, char *key_buf, usize key_size,
                 char *val_buf, usize val_size)
{
    char *env = GetEnvironmentStringsA();
    if (!env) { if (key_size > 0) key_buf[0] = '\0'; if (val_size > 0) val_buf[0] = '\0'; return; }
    const char *p = env;
    i32 i;
    for (i = 0; i < idx && *p; i++) { while (*p) p++; p++; }
    if (*p) {
        /* parse KEY=VALUE */
        const char *eq = p;
        while (*eq && *eq != '=') eq++;
        usize klen = (usize)(eq - p);
        if (klen >= key_size) klen = key_size - 1;
        usize j;
        for (j = 0; j < klen; j++) key_buf[j] = p[j];
        key_buf[j] = '\0';
        const char *vs = (*eq == '=') ? eq + 1 : "";
        usize vlen = (usize)cstr_len(vs);
        if (vlen >= val_size) vlen = val_size - 1;
        for (j = 0; j < vlen; j++) val_buf[j] = vs[j];
        val_buf[j] = '\0';
    } else {
        if (key_size > 0) key_buf[0] = '\0';
        if (val_size > 0) val_buf[0] = '\0';
    }
    FreeEnvironmentStringsA(env);
}

i32 env_hostname(char *buf, usize size)
{
    DWORD sz = (DWORD)size;
    return GetComputerNameA(buf, &sz) ? (i32)sz : -1;
}

i32 env_args_count(void)
{
    return g_argc;
}

i64 env_args_get(i32 idx, char *buf, usize size)
{
    if (idx < 0 || idx >= g_argc || !g_argv) {
        if (size > 0) buf[0] = '\0';
        return -1;
    }
    const char *arg = g_argv[idx];
    i32 len = cstr_len(arg);
    if ((usize)len >= size) len = (i32)(size - 1);
    i32 j;
    for (j = 0; j < len; j++) buf[j] = arg[j];
    buf[j] = '\0';
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
        i32 len = cstr_len(arg);
        char *buf = (char *)mem_alloc((usize)len + 1u);
        if (!buf) { str_slot_set(arr, i, (char *)0, 0); continue; }
        i32 j;
        for (j = 0; j < len; j++) buf[j] = arg[j];
        buf[j] = '\0';
        str_slot_set(arr, i, buf, len);
    }
}

void env_all_fill(char *arr, i32 count)
{
    char *env = GetEnvironmentStringsA();
    if (!env) return;
    const char *p = env;
    i32 i;
    for (i = 0; i < count && *p; i++) {
        i32 len = cstr_len(p);
        char *buf = (char *)mem_alloc((usize)len + 1u);
        if (!buf) { p += len + 1; continue; }
        i32 j;
        for (j = 0; j <= len; j++) buf[j] = p[j];
        str_slot_set(arr, i, buf, len);
        p += len + 1;
    }
    FreeEnvironmentStringsA(env);
}

i32 env_osname(char *buf, usize size)
{
    const char *name = "windows";
    usize len = 7u;
    if (len >= size) len = size - 1u;
    usize j;
    for (j = 0; j < len; j++) buf[j] = name[j];
    buf[j] = '\0';
    return (i32)len;
}
