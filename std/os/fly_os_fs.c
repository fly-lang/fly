/*===-- std/os/fly_os_fs.c - fly.os filesystem implementation -----------===*/

#include "fly_os_fs.h"
#include "fly_os_linux.h"

extern void *malloc (unsigned long);
extern void *realloc(void *, unsigned long);
extern void  free   (void *);

/* ── Internal helpers ────────────────────────────────────────────────────── */

#define PATH_MAX_LEN 4096
#define IO_BUF_SIZE  65536

static int cstr_len(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void cstr_copy(char *d, const char *s, int n) {
    for (int i = 0; i < n; i++) d[i] = s[i];
}

static int fstr_to_buf(const fly_string *s, char *buf, int cap) {
    if (!s || !s->ptr || s->size <= 0) { buf[0] = '\0'; return 0; }
    int n = (s->size < cap - 1) ? s->size : cap - 1;
    for (int i = 0; i < n; i++) buf[i] = s->ptr[i];
    buf[n] = '\0';
    return n;
}

static int open_flags_from_fly(uint8_t flags, int *oflags, int *must_create) {
    *oflags      = 0;
    *must_create = 0;
    int rd = flags & FLY_FILE_READ;
    int wr = (flags & FLY_FILE_WRITE) || (flags & FLY_FILE_APPEND);
    if (rd && wr) *oflags = O_RDWR;
    else if (wr)  *oflags = O_WRONLY;
    else          *oflags = O_RDONLY;
    if (flags & FLY_FILE_APPEND)  *oflags |= O_APPEND;
    if (flags & FLY_FILE_CREATE)  *oflags |= O_CREAT;
    if (flags & FLY_FILE_TRUNC)   *oflags |= O_TRUNC;
    if (flags & FLY_FILE_EXCL) { *oflags |= O_EXCL; *must_create = 1; }
    return *oflags;
}

static void fill_stat(linux_stat *ls, fly_stat *out) {
    out->size       = (uint64_t)ls->st_size;
    out->mtime_sec  = (uint64_t)ls->st_mtime;
    out->mtime_nsec = (uint64_t)ls->st_mtime_nsec;
    out->mode       = (uint32_t)ls->st_mode;
    out->is_file    = S_ISREG(ls->st_mode)  ? 1 : 0;
    out->is_dir     = S_ISDIR(ls->st_mode)  ? 1 : 0;
    out->is_symlink = S_ISLNK(ls->st_mode)  ? 1 : 0;
}

/* grow fly_buf to at least needed */
static void buf_grow(fly_buf *b, size_t needed) {
    if (b->cap >= needed) return;
    size_t newcap = b->cap ? b->cap * 2 : 4096;
    if (newcap < needed) newcap = needed;
    uint8_t *p = (uint8_t *)realloc(b->ptr, (unsigned long)newcap);
    if (!p) return;
    b->ptr = p;
    b->cap = newcap;
}

/* ── Reader/Writer backends for fly_file ─────────────────────────────────── */

static void fd_read(void *ctx, uint8_t *buf, size_t n, size_t *out_read) {
    int fd = (int)(long)ctx;
    long r = __os_sc3(SYS_read, (long)fd, (long)buf, (long)n);
    *out_read = (r > 0) ? (size_t)r : 0;
}
static void fd_close_r(void *ctx) { __os_sc1(SYS_close, (long)(int)(long)ctx); }

static void fd_write(void *ctx, const uint8_t *buf, size_t n, size_t *out_written) {
    int fd = (int)(long)ctx;
    long r = __os_sc3(SYS_write, (long)fd, (long)buf, (long)n);
    *out_written = (r > 0) ? (size_t)r : 0;
}
static void fd_flush (void *ctx) { (void)ctx; }
static void fd_close_w(void *ctx) { __os_sc1(SYS_close, (long)(int)(long)ctx); }

/* ══════════════════════════════════════════════════════════════════════════ */
/* Open / close                                                               */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_open(const fly_string *path, fly_file *out) {
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)buf, (long)O_RDONLY, 0L);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = FLY_FILE_READ;
}

void fly_fs_create(const fly_string *path, fly_file *out) {
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)buf,
                       (long)(O_WRONLY | O_CREAT | O_TRUNC), (long)0644);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = FLY_FILE_WRITE | FLY_FILE_CREATE | FLY_FILE_TRUNC;
}

void fly_fs_openOpts(const fly_string *path, uint8_t flags, uint32_t perm, fly_file *out) {
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    int oflags, must_create;
    open_flags_from_fly(flags, &oflags, &must_create);
    long fd = __os_sc3(SYS_open, (long)buf, (long)oflags, (long)perm);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = flags;
}

void fly_fs_close(fly_file *f) {
    if (f->fd >= 0) { __os_sc1(SYS_close, (long)f->fd); f->fd = -1; }
}

void fly_fs_reader(fly_file *f, fly_reader *out) {
    out->ctx   = (void *)(long)f->fd;
    out->read  = fd_read;
    out->close = fd_close_r;
}

void fly_fs_writer(fly_file *f, fly_writer *out) {
    out->ctx   = (void *)(long)f->fd;
    out->write = fd_write;
    out->flush = fd_flush;
    out->close = fd_close_w;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Convenience read / write                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_read(const fly_string *path, fly_buf *out) {
    out->ptr  = (uint8_t *)0;
    out->size = 0;
    out->cap  = 0;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)pbuf, (long)O_RDONLY, 0L);
    if (fd < 0) return;
    uint8_t tmp[IO_BUF_SIZE];
    while (1) {
        long n = __os_sc3(SYS_read, fd, (long)tmp, (long)sizeof(tmp));
        if (n <= 0) break;
        buf_grow(out, out->size + (size_t)n);
        if (!out->ptr) break;
        for (long i = 0; i < n; i++) out->ptr[out->size + (size_t)i] = tmp[i];
        out->size += (size_t)n;
    }
    __os_sc1(SYS_close, fd);
}

static void write_all_fd(int fd, const uint8_t *data, size_t len) {
    size_t written = 0;
    while (written < len) {
        long n = __os_sc3(SYS_write, (long)fd, (long)(data + written),
                          (long)(len - written));
        if (n <= 0) break;
        written += (size_t)n;
    }
}

void fly_fs_write(const fly_string *path, const fly_buf *data, uint32_t perm) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)pbuf,
                       (long)(O_WRONLY | O_CREAT | O_TRUNC), (long)perm);
    if (fd < 0) return;
    if (data && data->ptr && data->size > 0)
        write_all_fd((int)fd, data->ptr, data->size);
    __os_sc1(SYS_close, fd);
}

void fly_fs_append(const fly_string *path, const fly_buf *data, uint32_t perm) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)pbuf,
                       (long)(O_WRONLY | O_CREAT | O_APPEND), (long)perm);
    if (fd < 0) return;
    if (data && data->ptr && data->size > 0)
        write_all_fd((int)fd, data->ptr, data->size);
    __os_sc1(SYS_close, fd);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Seek                                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_seekTo(fly_file *f, int64_t offset, int whence, int64_t *out_pos) {
    long r = __os_sc3(SYS_lseek, (long)f->fd, (long)offset, (long)whence);
    *out_pos = (r >= 0) ? (int64_t)r : -1;
}

void fly_fs_seekPos(fly_file *f, int64_t *out_pos) {
    long r = __os_sc3(SYS_lseek, (long)f->fd, 0L, (long)FLY_SEEK_CUR);
    *out_pos = (r >= 0) ? (int64_t)r : -1;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Metadata                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_stat(const fly_string *path, fly_stat *out) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    long r = __os_sc2(SYS_stat, (long)pbuf, (long)&ls);
    if (r < 0) { out->size = 0; out->is_file = 0; out->is_dir = 0; out->is_symlink = 0; return; }
    fill_stat(&ls, out);
}

void fly_fs_lstat(const fly_string *path, fly_stat *out) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    long r = __os_sc2(SYS_lstat, (long)pbuf, (long)&ls);
    if (r < 0) { out->size = 0; out->is_file = 0; out->is_dir = 0; out->is_symlink = 0; return; }
    fill_stat(&ls, out);
}

void fly_fs_size(const fly_string *path, uint64_t *out) {
    fly_stat st;
    fly_fs_stat(path, &st);
    *out = st.size;
}

void fly_fs_exists(const fly_string *path, int *out) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    *out = (__os_sc2(SYS_stat, (long)pbuf, (long)&ls) >= 0) ? 1 : 0;
}

void fly_fs_sync(fly_file *f) {
    if (f->fd >= 0) __os_sc1(SYS_fsync, (long)f->fd);
}

void fly_fs_truncate(const fly_string *path, uint64_t size) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_truncate, (long)pbuf, (long)size);
}

void fly_fs_chmod(const fly_string *path, uint32_t mode) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_chmod, (long)pbuf, (long)mode);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* File operations                                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_delete(const fly_string *path) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc1(SYS_unlink, (long)pbuf);
}

void fly_fs_copy(const fly_string *src, const fly_string *dst) {
    fly_buf data = {(uint8_t *)0, 0, 0};
    fly_fs_read(src, &data);
    if (data.ptr) {
        fly_fs_write(dst, &data, 0644);
        free(data.ptr);
    }
}

void fly_fs_move(const fly_string *src, const fly_string *dst) {
    char sbuf[PATH_MAX_LEN], dbuf[PATH_MAX_LEN];
    fstr_to_buf(src, sbuf, PATH_MAX_LEN);
    fstr_to_buf(dst, dbuf, PATH_MAX_LEN);
    __os_sc2(SYS_rename, (long)sbuf, (long)dbuf);
}

void fly_fs_rename(const fly_string *src, const fly_string *dst) {
    fly_fs_move(src, dst);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Directory operations                                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_dirCreate(const fly_string *path, uint32_t perm) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_mkdir, (long)pbuf, (long)perm);
}

void fly_fs_dirCreateAll(const fly_string *path, uint32_t perm) {
    char pbuf[PATH_MAX_LEN];
    int n = fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    for (int i = 1; i <= n; i++) {
        if (i < n && pbuf[i] != '/') continue;
        char c = pbuf[i]; pbuf[i] = '\0';
        if (__os_sc2(SYS_stat, (long)pbuf, (long)&ls) < 0)
            __os_sc2(SYS_mkdir, (long)pbuf, (long)perm);
        pbuf[i] = c;
    }
}

void fly_fs_dirDelete(const fly_string *path) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc1(SYS_rmdir, (long)pbuf);
}

/* Recursive delete — simple iterative approach via depth-first walk */
void fly_fs_dirDeleteAll(const fly_string *path) {
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    if (__os_sc2(SYS_lstat, (long)pbuf, (long)&ls) < 0) return;
    if (!S_ISDIR(ls.st_mode)) { __os_sc1(SYS_unlink, (long)pbuf); return; }
    long fd = __os_sc3(SYS_open, (long)pbuf, (long)(O_RDONLY | O_DIRECTORY), 0L);
    if (fd < 0) return;
    char dbuf[4096];
    while (1) {
        long n = __os_sc3(SYS_getdents64, fd, (long)dbuf, (long)sizeof(dbuf));
        if (n <= 0) break;
        long pos = 0;
        while (pos < n) {
            linux_dirent64 *de = (linux_dirent64 *)(dbuf + pos);
            pos += de->d_reclen;
            const char *name = de->d_name;
            if (name[0]=='.'&&(name[1]=='\0'||(name[1]=='.'&&name[2]=='\0'))) continue;
            char child[PATH_MAX_LEN];
            int plen = cstr_len(pbuf), nlen = cstr_len(name);
            cstr_copy(child, pbuf, plen);
            child[plen] = '/';
            cstr_copy(child + plen + 1, name, nlen);
            child[plen + 1 + nlen] = '\0';
            fly_string cs = { child, plen + 1 + nlen };
            fly_fs_dirDeleteAll(&cs);
        }
    }
    __os_sc1(SYS_close, fd);
    __os_sc1(SYS_rmdir, (long)pbuf);
}

void fly_fs_dirRead(const fly_string *path, fly_dir_entries *out) {
    out->items = (fly_dir_entry *)0;
    out->len   = 0;
    out->cap   = 0;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)pbuf, (long)(O_RDONLY | O_DIRECTORY), 0L);
    if (fd < 0) return;
    char dbuf[4096];
    while (1) {
        long n = __os_sc3(SYS_getdents64, fd, (long)dbuf, (long)sizeof(dbuf));
        if (n <= 0) break;
        long pos = 0;
        while (pos < n) {
            linux_dirent64 *de = (linux_dirent64 *)(dbuf + pos);
            pos += de->d_reclen;
            const char *name = de->d_name;
            if (name[0]=='.'&&(name[1]=='\0'||(name[1]=='.'&&name[2]=='\0'))) continue;
            /* grow array */
            if (out->len >= out->cap) {
                size_t newcap = out->cap ? out->cap * 2 : 8;
                fly_dir_entry *p = (fly_dir_entry *)realloc(out->items,
                                    (unsigned long)newcap * sizeof(fly_dir_entry));
                if (!p) break;
                out->items = p;
                out->cap   = newcap;
            }
            fly_dir_entry *ent = &out->items[out->len++];
            int nlen = cstr_len(name);
            char *np = (char *)malloc((unsigned long)nlen);
            cstr_copy(np, name, nlen);
            ent->name.ptr  = np;
            ent->name.size = nlen;
            /* stat the entry */
            char child[PATH_MAX_LEN];
            int plen = cstr_len(pbuf);
            cstr_copy(child, pbuf, plen);
            child[plen] = '/';
            cstr_copy(child + plen + 1, name, nlen);
            child[plen + 1 + nlen] = '\0';
            linux_stat ls;
            if (__os_sc2(SYS_lstat, (long)child, (long)&ls) == 0)
                fill_stat(&ls, &ent->stat);
            else {
                ent->stat.size = 0;
                ent->stat.is_file = 0;
                ent->stat.is_dir  = 0;
                ent->stat.is_symlink = 0;
            }
        }
    }
    __os_sc1(SYS_close, fd);
}

void fly_fs_dirWalk(const fly_string *path,
                    void (*cb)(const fly_string *, const fly_stat *, void *),
                    void *userdata) {
    fly_dir_entries entries = {(fly_dir_entry *)0, 0, 0};
    fly_fs_dirRead(path, &entries);
    for (size_t i = 0; i < entries.len; i++) {
        fly_dir_entry *e = &entries.items[i];
        /* build child path */
        fly_string child;
        char *p = (char *)malloc(
            (unsigned long)(path->size + 1 + e->name.size));
        cstr_copy(p, path->ptr, path->size);
        p[path->size] = '/';
        cstr_copy(p + path->size + 1, e->name.ptr, e->name.size);
        child.ptr  = p;
        child.size = path->size + 1 + e->name.size;
        cb(&child, &e->stat, userdata);
        if (e->stat.is_dir) fly_fs_dirWalk(&child, cb, userdata);
        free(p);
        free(e->name.ptr);
    }
    if (entries.items) free(entries.items);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Symlinks                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_fs_symlinkCreate(const fly_string *target, const fly_string *link) {
    char tbuf[PATH_MAX_LEN], lbuf[PATH_MAX_LEN];
    fstr_to_buf(target, tbuf, PATH_MAX_LEN);
    fstr_to_buf(link,   lbuf, PATH_MAX_LEN);
    __os_sc2(SYS_symlink, (long)tbuf, (long)lbuf);
}

void fly_fs_symlinkRead(const fly_string *path, fly_string *out) {
    char pbuf[PATH_MAX_LEN], rbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long n = __os_sc3(SYS_readlink, (long)pbuf, (long)rbuf, (long)(PATH_MAX_LEN-1));
    if (n < 0) { out->ptr = (char *)0; out->size = 0; return; }
    rbuf[n] = '\0';
    char *p = (char *)malloc((unsigned long)n);
    cstr_copy(p, rbuf, (int)n);
    out->ptr  = p;
    out->size = (int)n;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Temporary files / directories                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

/* Simple counter-based unique name (NOT cryptographically safe) */
static unsigned int g_tempcount = 0;

static void u32_to_hex(unsigned int v, char *out) {
    const char hex[] = "0123456789abcdef";
    for (int i = 7; i >= 0; i--) { out[i] = hex[v & 0xf]; v >>= 4; }
}

static void build_temp_name(const fly_string *dir, const fly_string *pattern,
                             char *out, int *outlen) {
    char suffix[9];
    linux_timespec ts;
    __os_sc2(SYS_clock_gettime, (long)CLOCK_MONOTONIC, (long)&ts);
    unsigned int u = (unsigned int)((unsigned long)ts.tv_nsec ^ (unsigned long)ts.tv_sec) + g_tempcount++;
    u32_to_hex(u, suffix);
    suffix[8] = '\0';

    int n = 0;
    cstr_copy(out + n, dir->ptr, dir->size); n += dir->size;
    out[n++] = '/';
    cstr_copy(out + n, pattern->ptr, pattern->size); n += pattern->size;
    cstr_copy(out + n, suffix, 8); n += 8;
    out[n] = '\0';
    *outlen = n;
}

void fly_fs_tempFile(const fly_string *dir, const fly_string *pattern,
                     fly_string *out_path, fly_file *out_file) {
    char pbuf[PATH_MAX_LEN];
    int  plen;
    build_temp_name(dir, pattern, pbuf, &plen);
    long fd = __os_sc3(SYS_open, (long)pbuf,
                       (long)(O_RDWR | O_CREAT | O_EXCL), (long)0600);
    /* retry once on collision */
    if (fd < 0) {
        build_temp_name(dir, pattern, pbuf, &plen);
        fd = __os_sc3(SYS_open, (long)pbuf,
                      (long)(O_RDWR | O_CREAT | O_EXCL), (long)0600);
    }
    out_file->fd    = (fd >= 0) ? (int)fd : -1;
    out_file->flags = FLY_FILE_READ | FLY_FILE_WRITE | FLY_FILE_CREATE;
    char *pp = (char *)malloc((unsigned long)plen);
    cstr_copy(pp, pbuf, plen);
    out_path->ptr  = pp;
    out_path->size = plen;
}

void fly_fs_tempDir(const fly_string *dir, const fly_string *pattern, fly_string *out) {
    char pbuf[PATH_MAX_LEN];
    int  plen;
    build_temp_name(dir, pattern, pbuf, &plen);
    long r = __os_sc2(SYS_mkdir, (long)pbuf, (long)0700);
    if (r < 0) {
        build_temp_name(dir, pattern, pbuf, &plen);
        __os_sc2(SYS_mkdir, (long)pbuf, (long)0700);
    }
    char *pp = (char *)malloc((unsigned long)plen);
    cstr_copy(pp, pbuf, plen);
    out->ptr  = pp;
    out->size = plen;
}
