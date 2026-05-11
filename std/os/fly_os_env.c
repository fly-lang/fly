/*===-- std/os/fly_os_env.c - fly.os environment implementation ---------===*/

#include "fly_os_env.h"
#include "fly_os_linux.h"

extern void *malloc (unsigned long);
extern void *realloc(void *, unsigned long);
extern void  free   (void *);

/* Provided by libc at link time */
extern char **environ;
extern int setenv  (const char *name, const char *value, int overwrite);
extern int unsetenv(const char *name);

/* argv registered by fly_env_init() */
static int    g_argc = 0;
static char **g_argv = (char **)0;

/* ── Internal helpers ────────────────────────────────────────────────────── */

static int cstr_len(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void cstr_copy(char *dst, const char *src, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

static int fstr_to_buf(const fly_string *s, char *buf, int cap) {
    if (!s || !s->ptr || s->size <= 0) { buf[0] = '\0'; return 0; }
    int n = (s->size < cap - 1) ? s->size : cap - 1;
    for (int i = 0; i < n; i++) buf[i] = s->ptr[i];
    buf[n] = '\0';
    return n;
}

static void fstr_from_cstr(const char *s, int len, fly_string *out) {
    if (len <= 0) { out->ptr = (char *)0; out->size = 0; return; }
    char *p = (char *)malloc((unsigned long)len);
    cstr_copy(p, s, len);
    out->ptr  = p;
    out->size = len;
}

/* ══════════════════════════════════════════════════════════════════════════ */

void fly_env_init(int argc, char **argv) {
    g_argc = argc;
    g_argv = argv;
}

void _F6fly_os6envGet_Ss_Ss(void *err_ctx, const fly_string *key, fly_string *out) {
    (void)err_ctx;
    out->ptr  = (char *)0;
    out->size = 0;
    if (!key || key->size <= 0 || !environ) return;
    int klen = key->size;
    for (int i = 0; environ[i]; i++) {
        const char *e = environ[i];
        int match = 1;
        for (int j = 0; j < klen; j++) {
            if (e[j] != key->ptr[j]) { match = 0; break; }
        }
        if (match && e[klen] == '=') {
            const char *val = e + klen + 1;
            int vlen = cstr_len(val);
            fstr_from_cstr(val, vlen, out);
            return;
        }
    }
}

void _F6fly_os6envSet_Ss_Ss(void *err_ctx, const fly_string *key, const fly_string *value) {
    (void)err_ctx;
    char kbuf[256], vbuf[4096];
    fstr_to_buf(key,   kbuf, (int)sizeof(kbuf));
    fstr_to_buf(value, vbuf, (int)sizeof(vbuf));
    setenv(kbuf, vbuf, 1);
}

void _F6fly_os9envDelete_Ss(void *err_ctx, const fly_string *key) {
    (void)err_ctx;
    char kbuf[256];
    fstr_to_buf(key, kbuf, (int)sizeof(kbuf));
    unsetenv(kbuf);
}

void _F6fly_os6envAll_Cfly_string_array(void *err_ctx, fly_string_array *out) {
    (void)err_ctx;
    out->items = (fly_string *)0;
    out->count = 0;
    if (!environ) return;
    int n = 0;
    while (environ[n]) n++;
    fly_string *arr = (fly_string *)malloc((unsigned long)n * sizeof(fly_string));
    if (!arr) return;
    for (int i = 0; i < n; i++) {
        int len = cstr_len(environ[i]);
        fstr_from_cstr(environ[i], len, &arr[i]);
    }
    out->items = arr;
    out->count = n;
}

void _F6fly_os9envExpand_Ss_Ss(void *err_ctx, const fly_string *s, fly_string *out) {
    (void)err_ctx;
    if (!s || !s->ptr || s->size == 0) { out->ptr = (char *)0; out->size = 0; return; }
    char result[8192];
    int rlen = 0;
    int i = 0;
    while (i < s->size) {
        char c = s->ptr[i];
        if (c == '$' && i + 1 < s->size) {
            i++;
            int braced = (s->ptr[i] == '{');
            if (braced) i++;
            int start = i;
            while (i < s->size) {
                char d = s->ptr[i];
                if (braced) { if (d == '}') break; }
                else { if (!((d >= 'A' && d <= 'Z') || (d >= 'a' && d <= 'z') ||
                             (d >= '0' && d <= '9') || d == '_')) break; }
                i++;
            }
            int klen = i - start;
            if (braced && i < s->size) i++; /* skip '}' */
            if (klen > 0 && environ) {
                for (int j = 0; environ[j]; j++) {
                    const char *e = environ[j];
                    int match = 1;
                    for (int k = 0; k < klen; k++) {
                        if (e[k] != s->ptr[start + k]) { match = 0; break; }
                    }
                    if (match && e[klen] == '=') {
                        const char *val = e + klen + 1;
                        int vlen = cstr_len(val);
                        if (rlen + vlen < (int)sizeof(result)) {
                            cstr_copy(result + rlen, val, vlen);
                            rlen += vlen;
                        }
                        break;
                    }
                }
            }
        } else {
            if (rlen < (int)sizeof(result) - 1) result[rlen++] = c;
            i++;
        }
    }
    fstr_from_cstr(result, rlen, out);
}

void _F6fly_os9envCwdGet_Ss(void *err_ctx, fly_string *out) {
    (void)err_ctx;
    char buf[4096];
    long r = __os_sc2(SYS_getcwd, (long)buf, (long)sizeof(buf));
    if (r <= 0) { out->ptr = (char *)0; out->size = 0; return; }
    int len = (int)(r - 1); /* r includes the null terminator */
    fstr_from_cstr(buf, len, out);
}

void _F6fly_os9envCwdSet_Ss(void *err_ctx, const fly_string *path) {
    (void)err_ctx;
    char buf[4096];
    fstr_to_buf(path, buf, (int)sizeof(buf));
    __os_sc1(SYS_chdir, (long)buf);
}

void _F6fly_os11envArgsGet_Cfly_string_array(void *err_ctx, fly_string_array *out) {
    (void)err_ctx;
    out->items = (fly_string *)0;
    out->count = 0;
    if (g_argc <= 0 || !g_argv) return;
    fly_string *arr = (fly_string *)malloc(
        (unsigned long)g_argc * sizeof(fly_string));
    if (!arr) return;
    for (int i = 0; i < g_argc; i++) {
        int len = cstr_len(g_argv[i]);
        fstr_from_cstr(g_argv[i], len, &arr[i]);
    }
    out->items = arr;
    out->count = g_argc;
}

void _F6fly_os13envArgsCount_i(void *err_ctx, int *out) {
    (void)err_ctx;
    *out = g_argc;
}

void _F6fly_os11envHostname_Ss(void *err_ctx, fly_string *out) {
    (void)err_ctx;
    linux_utsname uts;
    long r = __os_sc1(SYS_uname, (long)&uts);
    if (r < 0) { out->ptr = (char *)0; out->size = 0; return; }
    int len = cstr_len(uts.nodename);
    fstr_from_cstr(uts.nodename, len, out);
}

void _F6fly_os9envOsname_Ss(void *err_ctx, fly_string *out) {
    (void)err_ctx;
#if defined(__linux__)
    fstr_from_cstr("linux", 5, out);
#elif defined(__APPLE__)
    fstr_from_cstr("macos", 5, out);
#elif defined(_WIN32)
    fstr_from_cstr("windows", 7, out);
#else
    fstr_from_cstr("unknown", 7, out);
#endif
}

void _F6fly_os7envExit_i(void *err_ctx, int code) {
    (void)err_ctx;
    __os_sc1(SYS_exit_group, (long)code);
    __builtin_unreachable();
}
