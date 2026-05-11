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

void _F6fly_os6fsOpen_Ss_Cfly_file(void *err_ctx, const fly_string *path, fly_file *out) {
    (void)err_ctx;
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)buf, (long)O_RDONLY, 0L);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = FLY_FILE_READ;
}

void _F6fly_os8fsCreate_Ss_Cfly_file(void *err_ctx, const fly_string *path, fly_file *out) {
    (void)err_ctx;
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)buf,
                       (long)(O_WRONLY | O_CREAT | O_TRUNC), (long)0644);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = FLY_FILE_WRITE | FLY_FILE_CREATE | FLY_FILE_TRUNC;
}

void _F6fly_os9fsOpenOpts_Ss_y_ui_Cfly_file(void *err_ctx, const fly_string *path, uint8_t flags, uint32_t perm, fly_file *out) {
    (void)err_ctx;
    char buf[PATH_MAX_LEN];
    fstr_to_buf(path, buf, PATH_MAX_LEN);
    int oflags, must_create;
    open_flags_from_fly(flags, &oflags, &must_create);
    long fd = __os_sc3(SYS_open, (long)buf, (long)oflags, (long)perm);
    out->fd    = (fd >= 0) ? (int)fd : -1;
    out->flags = flags;
}

void _F6fly_os7fsClose_Cfly_file(void *err_ctx, fly_file *f) {
    (void)err_ctx;
    if (f->fd >= 0) { __os_sc1(SYS_close, (long)f->fd); f->fd = -1; }
}

void _F6fly_os8fsReader_Cfly_file_Cfly_reader(void *err_ctx, fly_file *f, fly_reader *out) {
    (void)err_ctx;
    out->ctx   = (void *)(long)f->fd;
    out->read  = fd_read;
    out->close = fd_close_r;
}

void _F6fly_os8fsWriter_Cfly_file_Cfly_writer(void *err_ctx, fly_file *f, fly_writer *out) {
    (void)err_ctx;
    out->ctx   = (void *)(long)f->fd;
    out->write = fd_write;
    out->flush = fd_flush;
    out->close = fd_close_w;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Convenience read / write                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os6fsRead_Ss_Cfly_buf(void *err_ctx, const fly_string *path, fly_buf *out) {
    (void)err_ctx;
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

void _F6fly_os7fsWrite_Ss_Cfly_buf_ui(void *err_ctx, const fly_string *path, const fly_buf *data, uint32_t perm) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    long fd = __os_sc3(SYS_open, (long)pbuf,
                       (long)(O_WRONLY | O_CREAT | O_TRUNC), (long)perm);
    if (fd < 0) return;
    if (data && data->ptr && data->size > 0)
        write_all_fd((int)fd, data->ptr, data->size);
    __os_sc1(SYS_close, fd);
}

void _F6fly_os8fsAppend_Ss_Cfly_buf_ui(void *err_ctx, const fly_string *path, const fly_buf *data, uint32_t perm) {
    (void)err_ctx;
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

void _F6fly_os9fsSeekTo_Cfly_file_l_i_l(void *err_ctx, fly_file *f, int64_t offset, int whence, int64_t *out_pos) {
    (void)err_ctx;
    long r = __os_sc3(SYS_lseek, (long)f->fd, (long)offset, (long)whence);
    *out_pos = (r >= 0) ? (int64_t)r : -1;
}

void _F6fly_os9fsSeekPos_Cfly_file_l(void *err_ctx, fly_file *f, int64_t *out_pos) {
    (void)err_ctx;
    long r = __os_sc3(SYS_lseek, (long)f->fd, 0L, (long)FLY_SEEK_CUR);
    *out_pos = (r >= 0) ? (int64_t)r : -1;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Metadata                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os6fsStat_Ss_Cfly_stat(void *err_ctx, const fly_string *path, fly_stat *out) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    long r = __os_sc2(SYS_stat, (long)pbuf, (long)&ls);
    if (r < 0) { out->size = 0; out->is_file = 0; out->is_dir = 0; out->is_symlink = 0; return; }
    fill_stat(&ls, out);
}

void _F6fly_os7fsLstat_Ss_Cfly_stat(void *err_ctx, const fly_string *path, fly_stat *out) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    long r = __os_sc2(SYS_lstat, (long)pbuf, (long)&ls);
    if (r < 0) { out->size = 0; out->is_file = 0; out->is_dir = 0; out->is_symlink = 0; return; }
    fill_stat(&ls, out);
}

void _F6fly_os6fsSize_Ss_ul(void *err_ctx, const fly_string *path, uint64_t *out) {
    (void)err_ctx;
    fly_stat st;
    _F6fly_os6fsStat_Ss_Cfly_stat((void*)0, path, &st);
    *out = st.size;
}

void _F6fly_os8fsExists_Ss_b(void *err_ctx, const fly_string *path, int *out) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    linux_stat ls;
    *out = (__os_sc2(SYS_stat, (long)pbuf, (long)&ls) >= 0) ? 1 : 0;
}

void _F6fly_os6fsSync_Cfly_file(void *err_ctx, fly_file *f) {
    (void)err_ctx;
    if (f->fd >= 0) __os_sc1(SYS_fsync, (long)f->fd);
}

void _F6fly_os10fsTruncate_Ss_ul(void *err_ctx, const fly_string *path, uint64_t size) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_truncate, (long)pbuf, (long)size);
}

void _F6fly_os7fsChmod_Ss_ui(void *err_ctx, const fly_string *path, uint32_t mode) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_chmod, (long)pbuf, (long)mode);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* File operations                                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os8fsDelete_Ss(void *err_ctx, const fly_string *path) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc1(SYS_unlink, (long)pbuf);
}

void _F6fly_os6fsCopy_Ss_Ss(void *err_ctx, const fly_string *src, const fly_string *dst) {
    (void)err_ctx;
    fly_buf data = {(uint8_t *)0, 0, 0};
    _F6fly_os6fsRead_Ss_Cfly_buf((void*)0, src, &data);
    if (data.ptr) {
        _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, dst, &data, 0644);
        free(data.ptr);
    }
}

void _F6fly_os6fsMove_Ss_Ss(void *err_ctx, const fly_string *src, const fly_string *dst) {
    (void)err_ctx;
    char sbuf[PATH_MAX_LEN], dbuf[PATH_MAX_LEN];
    fstr_to_buf(src, sbuf, PATH_MAX_LEN);
    fstr_to_buf(dst, dbuf, PATH_MAX_LEN);
    __os_sc2(SYS_rename, (long)sbuf, (long)dbuf);
}

void _F6fly_os8fsRename_Ss_Ss(void *err_ctx, const fly_string *src, const fly_string *dst) {
    (void)err_ctx;
    _F6fly_os6fsMove_Ss_Ss((void*)0, src, dst);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Directory operations                                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os11fsDirCreate_Ss_ui(void *err_ctx, const fly_string *path, uint32_t perm) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc2(SYS_mkdir, (long)pbuf, (long)perm);
}

void _F6fly_os14fsDirCreateAll_Ss_ui(void *err_ctx, const fly_string *path, uint32_t perm) {
    (void)err_ctx;
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

void _F6fly_os11fsDirDelete_Ss(void *err_ctx, const fly_string *path) {
    (void)err_ctx;
    char pbuf[PATH_MAX_LEN];
    fstr_to_buf(path, pbuf, PATH_MAX_LEN);
    __os_sc1(SYS_rmdir, (long)pbuf);
}

/* Recursive delete — simple iterative approach via depth-first walk */
void _F6fly_os14fsDirDeleteAll_Ss(void *err_ctx, const fly_string *path) {
    (void)err_ctx;
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
            _F6fly_os14fsDirDeleteAll_Ss((void*)0, &cs);
        }
    }
    __os_sc1(SYS_close, fd);
    __os_sc1(SYS_rmdir, (long)pbuf);
}

void _F6fly_os9fsDirRead_Ss_Cfly_dir_entries(void *err_ctx, const fly_string *path, fly_dir_entries *out) {
    (void)err_ctx;
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
            linux_stat lss;
            if (__os_sc2(SYS_lstat, (long)child, (long)&lss) == 0)
                fill_stat(&lss, &ent->stat);
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

void _F6fly_os9fsDirWalk_Ss_Cfly_walk_fn(void *err_ctx, const fly_string *path,
                    void (*cb)(const fly_string *, const fly_stat *, void *),
                    void *userdata) {
    (void)err_ctx;
    fly_dir_entries entries = {(fly_dir_entry *)0, 0, 0};
    _F6fly_os9fsDirRead_Ss_Cfly_dir_entries((void*)0, path, &entries);
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
        if (e->stat.is_dir) _F6fly_os9fsDirWalk_Ss_Cfly_walk_fn((void*)0, &child, cb, userdata);
        free(p);
        free(e->name.ptr);
    }
    if (entries.items) free(entries.items);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Symlinks                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os15fsSymlinkCreate_Ss_Ss(void *err_ctx, const fly_string *target, const fly_string *link) {
    (void)err_ctx;
    char tbuf[PATH_MAX_LEN], lbuf[PATH_MAX_LEN];
    fstr_to_buf(target, tbuf, PATH_MAX_LEN);
    fstr_to_buf(link,   lbuf, PATH_MAX_LEN);
    __os_sc2(SYS_symlink, (long)tbuf, (long)lbuf);
}

void _F6fly_os13fsSymlinkRead_Ss_Ss(void *err_ctx, const fly_string *path, fly_string *out) {
    (void)err_ctx;
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

void _F6fly_os10fsTempFile_Ss_Ss_Ss_Cfly_file(void *err_ctx, const fly_string *dir, const fly_string *pattern,
                     fly_string *out_path, fly_file *out_file) {
    (void)err_ctx;
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

void _F6fly_os9fsTempDir_Ss_Ss_Ss(void *err_ctx, const fly_string *dir, const fly_string *pattern, fly_string *out) {
    (void)err_ctx;
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
