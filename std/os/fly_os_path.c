/*===-- std/os/fly_os_path.c - fly.os path manipulation -----------------===
 *
 * Pure string logic — no syscalls except absolute (getcwd),
 * isFile/isDir/isSym (stat), and glob (opendir+readdir).
 *===----------------------------------------------------------------------===*/

#include "fly_os_path.h"
#include "fly_os_linux.h"

extern void *malloc (unsigned long);
extern void *realloc(void *, unsigned long);
extern void  free   (void *);

/* ── Internal helpers ────────────────────────────────────────────────────── */

#define PATH_SEP '/'
#define PATH_MAX_LEN 4096

/* Copy fly_string into a null-terminated stack buffer. Returns written bytes. */
static int fstr_to_buf(const fly_string *s, char *buf, int cap) {
    if (!s || !s->ptr || s->size <= 0) { buf[0] = '\0'; return 0; }
    int n = (s->size < cap - 1) ? s->size : cap - 1;
    for (int i = 0; i < n; i++) buf[i] = s->ptr[i];
    buf[n] = '\0';
    return n;
}

/* Build fly_string from a C string (heap copy). */
static void fstr_from_cstr(const char *s, int len, fly_string *out) {
    if (len <= 0) { out->ptr = (char *)0; out->size = 0; return; }
    char *p = (char *)malloc((unsigned long)len);
    for (int i = 0; i < len; i++) p[i] = s[i];
    out->ptr  = p;
    out->size = len;
}

static int cstr_len(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}

static void cstr_copy(char *dst, const char *src, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

/* ── fly_string_array builder ────────────────────────────────────────────── */

typedef struct { fly_string *items; int count; int cap; } sa_builder;

static void sa_push(sa_builder *b, fly_string s) {
    if (b->count >= b->cap) {
        int newcap = b->cap ? b->cap * 2 : 8;
        fly_string *p = (fly_string *)realloc(b->items,
                            (unsigned long)newcap * sizeof(fly_string));
        if (!p) return;
        b->items = p;
        b->cap   = newcap;
    }
    b->items[b->count++] = s;
}

/* ── Glob / fnmatch ──────────────────────────────────────────────────────── */

/* Returns 1 if name matches pattern.  Supports *, ?, [] character classes. */
static int glob_match(const char *pat, int plen, const char *name, int nlen) {
    int p = 0, n = 0;
    int star_p = -1, star_n = -1;
    while (n < nlen) {
        if (p < plen && pat[p] == '*') {
            star_p = p++;
            star_n = n;
        } else if (p < plen && (pat[p] == '?' || pat[p] == name[n])) {
            p++; n++;
        } else if (p < plen && pat[p] == '[') {
            /* character class */
            p++;
            int neg = (p < plen && pat[p] == '!') ? (p++, 1) : 0;
            int matched = 0;
            while (p < plen && pat[p] != ']') {
                if (p + 2 < plen && pat[p+1] == '-' && pat[p+2] != ']') {
                    matched |= (name[n] >= pat[p] && name[n] <= pat[p+2]);
                    p += 3;
                } else {
                    matched |= (name[n] == pat[p]);
                    p++;
                }
            }
            if (p < plen) p++; /* skip ']' */
            if (matched == neg) goto backtrack;
            n++;
        } else {
        backtrack:
            if (star_p < 0) return 0;
            p  = star_p + 1;
            n  = ++star_n;
        }
    }
    while (p < plen && pat[p] == '*') p++;
    return p == plen;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Public API                                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_path_sep(uint8_t *out) { *out = (uint8_t)PATH_SEP; }

void fly_path_isAbsolute(const fly_string *path, int *out) {
    *out = (path && path->size > 0 && path->ptr[0] == PATH_SEP);
}

void fly_path_isRelative(const fly_string *path, int *out) {
    int abs; fly_path_isAbsolute(path, &abs); *out = !abs;
}

void fly_path_join(const fly_string *base, const fly_string *comp, fly_string *out) {
    if (!base || base->size == 0) { *out = *comp; return; }
    if (!comp || comp->size == 0) { *out = *base; return; }
    /* remove trailing '/' from base */
    int blen = base->size;
    while (blen > 1 && base->ptr[blen-1] == PATH_SEP) blen--;
    /* remove leading '/' from comp if base is non-empty */
    const char *cp = comp->ptr;
    int clen = comp->size;
    if (clen > 0 && cp[0] == PATH_SEP) { cp++; clen--; }
    int total = blen + 1 + clen;
    char *p = (char *)malloc((unsigned long)total);
    cstr_copy(p, base->ptr, blen);
    p[blen] = PATH_SEP;
    cstr_copy(p + blen + 1, cp, clen);
    out->ptr  = p;
    out->size = total;
}

void fly_path_joinN(const fly_string *parts, size_t n, fly_string *out) {
    if (n == 0) { out->ptr = (char *)0; out->size = 0; return; }
    fly_string cur = parts[0];
    for (size_t i = 1; i < n; i++) {
        fly_string next;
        fly_path_join(&cur, &parts[i], &next);
        if (i > 1 && cur.ptr) free(cur.ptr); /* free intermediates (not first) */
        cur = next;
    }
    *out = cur;
}

void fly_path_basename(const fly_string *path, fly_string *out) {
    if (!path || path->size == 0) { out->ptr = (char *)0; out->size = 0; return; }
    int end = path->size;
    /* strip trailing slashes (but keep root "/") */
    while (end > 1 && path->ptr[end-1] == PATH_SEP) end--;
    int start = end - 1;
    while (start > 0 && path->ptr[start-1] != PATH_SEP) start--;
    int len = end - start;
    fstr_from_cstr(path->ptr + start, len, out);
}

void fly_path_dirname(const fly_string *path, fly_string *out) {
    if (!path || path->size == 0) { fstr_from_cstr(".", 1, out); return; }
    int end = path->size;
    while (end > 1 && path->ptr[end-1] == PATH_SEP) end--;
    int pos = end - 1;
    while (pos > 0 && path->ptr[pos] != PATH_SEP) pos--;
    if (pos == 0) {
        /* either root "/" or current dir "." */
        if (path->ptr[0] == PATH_SEP) fstr_from_cstr("/", 1, out);
        else                           fstr_from_cstr(".", 1, out);
        return;
    }
    while (pos > 1 && path->ptr[pos-1] == PATH_SEP) pos--;
    fstr_from_cstr(path->ptr, pos, out);
}

void fly_path_ext(const fly_string *path, fly_string *out) {
    fly_string base;
    fly_path_basename(path, &base);
    if (!base.ptr || base.size == 0) { out->ptr = (char *)0; out->size = 0; return; }
    int dot = -1;
    for (int i = base.size - 1; i > 0; i--) {
        if (base.ptr[i] == '.') { dot = i; break; }
    }
    if (dot < 0) { out->ptr = (char *)0; out->size = 0; }
    else         { fstr_from_cstr(base.ptr + dot, base.size - dot, out); }
    free(base.ptr);
}

void fly_path_stem(const fly_string *path, fly_string *out) {
    fly_string base;
    fly_path_basename(path, &base);
    if (!base.ptr || base.size == 0) { out->ptr = (char *)0; out->size = 0; return; }
    int dot = -1;
    for (int i = base.size - 1; i > 0; i--) {
        if (base.ptr[i] == '.') { dot = i; break; }
    }
    if (dot < 0) { *out = base; }
    else         { fstr_from_cstr(base.ptr, dot, out); free(base.ptr); }
}

void fly_path_split(const fly_string *path, fly_string *out_dir, fly_string *out_base) {
    fly_path_dirname(path, out_dir);
    fly_path_basename(path, out_base);
}

void fly_path_splitExt(const fly_string *path, fly_string *out_stem, fly_string *out_ext) {
    fly_path_stem(path, out_stem);
    fly_path_ext(path, out_ext);
}

/* Normalize path: collapse //, resolve . and .. */
void fly_path_normalize(const fly_string *path, fly_string *out) {
    if (!path || path->size == 0) { fstr_from_cstr(".", 1, out); return; }
    char buf[PATH_MAX_LEN];
    int blen = 0;
    int abs = (path->ptr[0] == PATH_SEP);
    /* split on '/' and process components */
    char comps[64][256];
    int nc = 0;
    int i = 0;
    while (i < path->size) {
        while (i < path->size && path->ptr[i] == PATH_SEP) i++;
        int start = i;
        while (i < path->size && path->ptr[i] != PATH_SEP) i++;
        int clen = i - start;
        if (clen == 0 || (clen == 1 && path->ptr[start] == '.')) continue;
        if (clen == 2 && path->ptr[start] == '.' && path->ptr[start+1] == '.') {
            if (nc > 0 && !(comps[nc-1][0] == '.' && comps[nc-1][1] == '\0'))
                nc--;
            else if (!abs) {
                if (nc < 64) { comps[nc][0] = '.'; comps[nc][1] = '.'; comps[nc][2] = '\0'; nc++; }
            }
        } else {
            if (nc < 64 && clen < 255) {
                for (int j = 0; j < clen; j++) comps[nc][j] = path->ptr[start+j];
                comps[nc][clen] = '\0';
                nc++;
            }
        }
    }
    if (!abs && nc == 0) { fstr_from_cstr(".", 1, out); return; }
    for (int c = 0; c < nc; c++) {
        if (c > 0 || abs) buf[blen++] = PATH_SEP;
        int l = cstr_len(comps[c]);
        cstr_copy(buf + blen, comps[c], l);
        blen += l;
    }
    if (blen == 0) { buf[blen++] = PATH_SEP; }
    fstr_from_cstr(buf, blen, out);
}

void fly_path_normalizeInPlace(fly_string *path) {
    fly_string tmp;
    fly_path_normalize(path, &tmp);
    free(path->ptr);
    *path = tmp;
}

void fly_path_absolute(const fly_string *path, fly_string *out) {
    if (path && path->size > 0 && path->ptr[0] == PATH_SEP) {
        fly_path_normalize(path, out);
        return;
    }
    /* prepend cwd */
    char cwd[PATH_MAX_LEN];
    long r = __os_sc2(SYS_getcwd, (long)cwd, (long)PATH_MAX_LEN);
    int cwdlen = (r > 0) ? (int)(r - 1) : 0; /* r includes null terminator */
    fly_string cwds = { cwd, cwdlen };
    fly_string joined;
    fly_path_join(&cwds, path, &joined);
    fly_path_normalize(&joined, out);
    free(joined.ptr);
}

void fly_path_rel(const fly_string *base, const fly_string *target, fly_string *out) {
    char bpath[PATH_MAX_LEN], tpath[PATH_MAX_LEN];
    fstr_to_buf(base, bpath, PATH_MAX_LEN);
    fstr_to_buf(target, tpath, PATH_MAX_LEN);
    /* find common prefix length (component-aligned) */
    int blen = cstr_len(bpath), tlen = cstr_len(tpath);
    int common = 0, last_sep = 0;
    while (common < blen && common < tlen && bpath[common] == tpath[common]) {
        if (bpath[common] == PATH_SEP) last_sep = common;
        common++;
    }
    if (common < blen && bpath[common] != PATH_SEP) common = last_sep;
    if (common < tlen && tpath[common] != PATH_SEP) common = last_sep;
    /* count ".." for remaining base components */
    char result[PATH_MAX_LEN];
    int rlen = 0;
    int i = common;
    while (i < blen) {
        if (bpath[i] == PATH_SEP) {
            if (rlen > 0) result[rlen++] = PATH_SEP;
            result[rlen++] = '.'; result[rlen++] = '.';
        }
        i++;
    }
    /* append remaining target */
    int tstart = common;
    if (tstart < tlen && tpath[tstart] == PATH_SEP) tstart++;
    if (tstart < tlen) {
        if (rlen > 0) result[rlen++] = PATH_SEP;
        int rem = tlen - tstart;
        cstr_copy(result + rlen, tpath + tstart, rem);
        rlen += rem;
    }
    if (rlen == 0) { result[0] = '.'; rlen = 1; }
    fstr_from_cstr(result, rlen, out);
}

/* stat-based checks */
static int do_lstat_mode(const fly_string *path, unsigned int *mode_out) {
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    linux_stat st;
    long r = __os_sc2(SYS_lstat, (long)buf, (long)&st);
    if (r < 0) return 0;
    *mode_out = st.st_mode;
    return 1;
}

void fly_path_isFile(const fly_string *path, int *out) {
    unsigned int m = 0;
    *out = do_lstat_mode(path, &m) && S_ISREG(m);
}

void fly_path_isDir(const fly_string *path, int *out) {
    unsigned int m = 0;
    *out = do_lstat_mode(path, &m) && S_ISDIR(m);
}

void fly_path_isSym(const fly_string *path, int *out) {
    unsigned int m = 0;
    *out = do_lstat_mode(path, &m) && S_ISLNK(m);
}

void fly_path_comp(const fly_string *path, fly_string_array *out) {
    sa_builder b = {(fly_string *)0, 0, 0};
    if (!path || path->size == 0) goto done;
    int i = 0;
    if (path->ptr[0] == PATH_SEP) {
        fly_string root; fstr_from_cstr("/", 1, &root);
        sa_push(&b, root);
        i = 1;
    }
    while (i < path->size) {
        while (i < path->size && path->ptr[i] == PATH_SEP) i++;
        int start = i;
        while (i < path->size && path->ptr[i] != PATH_SEP) i++;
        int clen = i - start;
        if (clen > 0) {
            fly_string comp; fstr_from_cstr(path->ptr + start, clen, &comp);
            sa_push(&b, comp);
        }
    }
done:
    out->items = b.items;
    out->count = b.count;
}

void fly_path_match(const fly_string *pattern, const fly_string *name, int *out) {
    if (!pattern || !name) { *out = 0; return; }
    *out = glob_match(pattern->ptr, pattern->size, name->ptr, name->size);
}

/* Glob: enumerate directory matching pattern like a glob expression */
void fly_path_glob(const fly_string *pattern, fly_string_array *out) {
    out->items = (fly_string *)0; out->count = 0;
    if (!pattern || pattern->size == 0) return;
    /* split pattern into dir + filename pattern */
    fly_string dir, filepat;
    fly_path_split(pattern, &dir, &filepat);
    char dirpath[PATH_MAX_LEN];
    fstr_to_buf(&dir, dirpath, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)dirpath,
                       (long)(O_RDONLY | O_DIRECTORY | O_CLOEXEC), 0L);
    if (fd < 0) { free(dir.ptr); free(filepat.ptr); return; }
    sa_builder b = {(fly_string *)0, 0, 0};
    char dbuf[4096];
    while (1) {
        long n = __os_sc3(SYS_getdents64, fd, (long)dbuf, (long)sizeof(dbuf));
        if (n <= 0) break;
        long pos = 0;
        while (pos < n) {
            linux_dirent64 *de = (linux_dirent64 *)(dbuf + pos);
            pos += de->d_reclen;
            if (de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
                continue;
            int nlen = cstr_len(de->d_name);
            if (!glob_match(filepat.ptr, filepat.size, de->d_name, nlen)) continue;
            /* build full path */
            fly_string fname = { de->d_name, nlen };
            fly_string full;
            fly_path_join(&dir, &fname, &full);
            sa_push(&b, full);
        }
    }
    __os_sc1(SYS_close, fd);
    free(dir.ptr);
    free(filepat.ptr);
    out->items = b.items;
    out->count = b.count;
}
