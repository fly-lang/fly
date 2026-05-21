/*===-- runtime/Windows/Fs.c - Filesystem primitives via UCRT/Win32 ------===*/

#include "../Runtime.h"
#include "Win32.h"

/* Map POSIX-style O_* flags to UCRT _open flags */
static int posix_flags_to_win(i32 flags)
{
    int wf = 0;
    int rw = flags & 3; /* O_RDONLY=0, O_WRONLY=1, O_RDWR=2 */
    if (rw == 1)       wf  = WIN_O_WRONLY;
    else if (rw == 2)  wf  = WIN_O_RDWR;
    else               wf  = WIN_O_RDONLY;
    if (flags & 64)    wf |= WIN_O_CREAT;
    if (flags & 512)   wf |= WIN_O_TRUNC;
    if (flags & 1024)  wf |= WIN_O_APPEND;
    return wf;
}

/* ── Directory-fd emulation ──────────────────────────────────────────────────
 *
 * Windows cannot open directories with _open(); we emulate the Linux-style
 * open(dir, O_RDONLY|O_DIRECTORY) + getdents64 idiom used by the Fly stdlib
 * with a small static table and FindFirstFileA / FindNextFileA.
 *
 * Virtual fds occupy the range [DIR_FD_BASE, DIR_FD_BASE + MAX_DIR_FDS).
 * Real CRT fds are always < DIR_FD_BASE in normal usage.
 *
 * linux_dirent64 layout produced (all Fly dents code expects this):
 *   +0  ino   (u64, 8 bytes) — always 0
 *   +8  off   (u64, 8 bytes) — always 0
 *   +16 reclen(u16 LE)       — total record size, 8-byte aligned
 *   +18 dtype (u8)           — 4=DT_DIR, 8=DT_REG
 *   +19 name  (null-terminated)
 */

#define DIR_FD_BASE   4096
#define MAX_DIR_FDS   16
#define DIR_PATH_MAX  4096

typedef struct {
    HANDLE           hFind;      /* INVALID_HANDLE = FindFirstFileA not called yet */
    WIN32_FIND_DATAA cur;        /* entry returned by last Find* call              */
    int              cur_ready;  /* 1 = cur holds an entry not yet written to buf  */
    int              done;       /* 1 = FindNextFileA returned FALSE (no more)     */
    int              in_use;     /* 1 = slot is occupied                           */
    char             path[DIR_PATH_MAX];
} DirCtx;

/* Global table — zero-initialised (in_use=0, hFind=NULL) at program start */
static DirCtx s_dirs[MAX_DIR_FDS];

/* Return 0-based slot index for a virtual fd, -1 if out of range or unused. */
static int dir_slot(i32 fd)
{
    int s = (int)fd - DIR_FD_BASE;
    if (s < 0 || s >= MAX_DIR_FDS) return -1;
    return s_dirs[s].in_use ? s : -1;
}

/* Claim a free slot; copy path; return virtual fd, or -1 on table full. */
static i32 dir_open(const char *path)
{
    for (int i = 0; i < MAX_DIR_FDS; i++) {
        if (!s_dirs[i].in_use) {
            DirCtx *c    = &s_dirs[i];
            c->hFind      = INVALID_HANDLE;
            c->cur_ready  = 0;
            c->done       = 0;
            c->in_use     = 1;
            int k = 0;
            while (path[k] && k < DIR_PATH_MAX - 1) { c->path[k] = path[k]; k++; }
            c->path[k] = '\0';
            return (i32)(DIR_FD_BASE + i);
        }
    }
    return -1; /* table full */
}

/* Build FindFirstFileA search pattern: path + "/" + "*" */
static void make_pattern(char *pat, const char *dir, int pat_max)
{
    int i = 0;
    while (dir[i] && i < pat_max - 3) { pat[i] = dir[i]; i++; }
    char last = (i > 0) ? pat[i - 1] : '\0';
    if (last != '/' && last != '\\') pat[i++] = '/';
    pat[i++] = '*';
    pat[i]   = '\0';
}

/* Write one linux_dirent64-compatible record to buf[pos..].
 * Returns the reclen (bytes consumed) on success, 0 if the record does not
 * fit in the remaining buffer space. */
static int write_dirent(char *buf, int pos, int buf_sz,
                         const char *name, int is_dir)
{
    int nlen = 0;
    while (name[nlen]) nlen++;

    /* raw = 8(ino)+8(off)+2(reclen)+1(dtype)+nlen+1(nul) = 20+nlen */
    int raw    = 20 + nlen;
    int reclen = (raw + 7) & ~7;   /* round up to 8-byte boundary */

    if (pos + reclen > buf_sz) return 0;

    for (int k = 0; k < reclen; k++) buf[pos + k] = 0; /* zero record */

    /* reclen at offset 16, little-endian u16 */
    buf[pos + 16] = (char)(reclen & 0xFF);
    buf[pos + 17] = (char)((reclen >> 8) & 0xFF);

    /* dtype at offset 18 */
    buf[pos + 18] = (char)(is_dir ? 4 : 8);

    /* name at offset 19 */
    for (int k = 0; k < nlen; k++) buf[pos + 19 + k] = name[k];

    return reclen;
}

/* ── fs_open ─────────────────────────────────────────────────────────────── */

i32 fs_open(const char *path, i32 flags, i32 mode)
{
    /* O_DIRECTORY (Linux value 65536): caller wants a dir fd for getdents */
    if (flags & 65536) {
        return dir_open(path);
    }

    /* Detect directories BEFORE calling _open.
     *
     * On Linux, open(dir, O_RDONLY) is the idiom for a getdents-capable fd.
     * On Windows, _open on a directory may either fail (EACCES) or succeed
     * with a CRT fd that cannot be used for getdents.  We handle both by
     * checking the path attributes first and returning a virtual dir fd. */
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (GetFileAttributesExA(path, 0, &info) &&
        (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return dir_open(path);
    }

    /* Regular file open */
    int r = _open(path, posix_flags_to_win(flags), (int)mode);
    return (r >= 0) ? (i32)r : -1;
}

/* ── fs_close ────────────────────────────────────────────────────────────── */

i32 fs_close(i32 fd)
{
    int slot = dir_slot(fd);
    if (slot >= 0) {
        DirCtx *c = &s_dirs[slot];
        if (c->hFind != INVALID_HANDLE) {
            FindClose(c->hFind);
            c->hFind = INVALID_HANDLE;
        }
        c->in_use = 0;
        return 0;
    }
    return (_close((int)fd) == 0) ? 0 : -1;
}

/* ── fs_read / fs_write / fs_seek ────────────────────────────────────────── */

i64 fs_read(i32 fd, void *buf, usize count)
{
    uint_rt c = (count > 0x7FFFFFFFU) ? 0x7FFFFFFFU : (uint_rt)count;
    int r = _read((int)fd, buf, c);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_write(i32 fd, const void *buf, usize count)
{
    uint_rt c = (count > 0x7FFFFFFFU) ? 0x7FFFFFFFU : (uint_rt)count;
    int r = _write((int)fd, buf, c);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_seek(i32 fd, i64 offset, i32 whence)
{
    long long r = _lseeki64((int)fd, (long long)offset, (int)whence);
    return (r >= 0) ? (i64)r : -1;
}

/* ── fs_stat / fs_mode / fs_size ─────────────────────────────────────────── */

i32 fs_stat(const char *path, u64 *size_out, u32 *mode_out)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path, 0 /* GetFileExInfoStandard */, &info)) {
        *size_out = 0; *mode_out = 0; return -1;
    }
    *size_out = ((u64)info.nFileSizeHigh << 32) | (u64)info.nFileSizeLow;
    /* Emulate POSIX mode bits minimally: S_IFDIR=0040000, S_IFREG=0100000 */
    *mode_out = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ? 0040755u : 0100644u;
    return 0;
}

/* Single-value stat helpers (Fly-compatible: one return value each) */
i64 fs_size(const char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path, 0, &info)) return -1;
    return (i64)(((u64)info.nFileSizeHigh << 32) | (u64)info.nFileSizeLow);
}

i32 fs_mode(const char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path, 0, &info)) return -1;
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0040755 : 0100644;
}

/* ── fs_lstat / fs_lsize / fs_lmode (symlink-aware) ─────────────────────── *
 *
 * GetFileAttributesExA follows symbolic links (returns target attributes).
 * FindFirstFileA does NOT follow symbolic links — it returns the symlink's
 * own attributes, including FILE_ATTRIBUTE_REPARSE_POINT.  We use that for
 * the lstat/lmode family.
 */

i32 fs_lstat(const char *path, u64 *size_out, u32 *mode_out)
{
    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(path, &ffd);
    if (h == INVALID_HANDLE) { *size_out = 0; *mode_out = 0; return -1; }
    FindClose(h);
    *size_out = ((u64)ffd.nFileSizeHigh << 32) | (u64)ffd.nFileSizeLow;
    if      (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) *mode_out = 0120755u; /* S_IFLNK */
    else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)     *mode_out = 0040755u; /* S_IFDIR */
    else                                                           *mode_out = 0100644u; /* S_IFREG */
    return 0;
}

i64 fs_lsize(const char *path)
{
    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(path, &ffd);
    if (h == INVALID_HANDLE) return -1;
    FindClose(h);
    return (i64)(((u64)ffd.nFileSizeHigh << 32) | (u64)ffd.nFileSizeLow);
}

i32 fs_lmode(const char *path)
{
    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(path, &ffd);
    if (h == INVALID_HANDLE) return -1;
    FindClose(h);
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) return 0120755; /* S_IFLNK */
    return (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0040755 : 0100644;
}

/* ── fs_mkdir / fs_unlink / fs_rename / fs_rmdir / fs_fsync ─────────────── */

i32 fs_mkdir(const char *path, i32 mode)
{
    (void)mode;
    return CreateDirectoryA(path, WIN32_NULL) ? 0 : -1;
}

i32 fs_unlink(const char *path)
{
    return DeleteFileA(path) ? 0 : -1;
}

i32 fs_rename(const char *src, const char *dst)
{
    return MoveFileExA(src, dst, MOVEFILE_REPLACE_EXISTING) ? 0 : -1;
}

i32 fs_rmdir(const char *path)
{
    return RemoveDirectoryA(path) ? 0 : -1;
}

i32 fs_fsync(i32 fd)
{
    (void)fd;
    return 0; /* no-op on Windows in this minimal runtime */
}

/* ── fs_truncate / fs_chmod ──────────────────────────────────────────────── */

i32 fs_truncate(const char *path, i64 size)
{
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, WIN32_NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, WIN32_NULL);
    if (h == INVALID_HANDLE) return -1;
    LARGE_INTEGER_rt li;
    li.QuadPart = (long long)size;
    if (!SetFilePointerEx(h, li, WIN32_NULL, FILE_BEGIN) || !SetEndOfFile(h)) {
        CloseHandle(h);
        return -1;
    }
    CloseHandle(h);
    return 0;
}

i32 fs_chmod(const char *path, i32 mode)
{
    /* Minimal: clear/set read-only based on write bit */
    DWORD attr = (mode & 0200) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_READONLY;
    return SetFileAttributesA(path, attr) ? 0 : -1;
}

/* ── fs_symlink / fs_readlink ────────────────────────────────────────────── */

i32 fs_symlink(const char *target, const char *link)
{
    /* Try unprivileged flag first (Developer Mode, Windows 10 1703+) */
    if (CreateSymbolicLinkA(link, target, 2 /* SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE */))
        return 0;
    /* Fall back to admin-required path */
    if (CreateSymbolicLinkA(link, target, 0))
        return 0;
    return -1;
}

i32 fs_readlink(const char *path, char *buf, usize size)
{
    (void)path; (void)buf; (void)size;
    return -1; /* not yet implemented on Windows */
}

/* ── fs_getdents ─────────────────────────────────────────────────────────── */

i32 fs_getdents(i32 fd, char *buf, usize size)
{
    int slot = dir_slot(fd);
    if (slot < 0) return -1;

    DirCtx *c = &s_dirs[slot];

    /* Already exhausted — tell caller we're done */
    if (c->done && !c->cur_ready) return 0;

    int total = 0;

    for (;;) {
        /* ── Step 1: write the pending entry (if any) ── */
        if (c->cur_ready) {
            const char *name = c->cur.cFileName;

            /* Skip "." and ".." */
            int is_dot = (name[0] == '.' &&
                          (name[1] == '\0' ||
                           (name[1] == '.' && name[2] == '\0')));
            if (is_dot) {
                c->cur_ready = 0;
                /* fall through to fetch next entry */
            } else {
                int is_dir = (c->cur.dwFileAttributes &
                              FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
                int w = write_dirent(buf, total, (int)size, name, is_dir);
                if (w == 0) return total; /* buffer full; entry stays pending */
                total      += w;
                c->cur_ready = 0;
                /* fall through to fetch next entry */
            }
        }

        /* ── Step 2: done? ── */
        if (c->done) break;

        /* ── Step 3: fetch next entry ── */
        if (c->hFind == INVALID_HANDLE) {
            /* First call: start enumeration */
            char pat[DIR_PATH_MAX + 4];
            make_pattern(pat, c->path, (int)sizeof(pat));
            c->hFind = FindFirstFileA(pat, &c->cur);
            if (c->hFind == INVALID_HANDLE) {
                c->done = 1;
                return total; /* empty directory or path not found */
            }
            c->cur_ready = 1;
        } else {
            /* Subsequent calls: advance to next entry */
            if (FindNextFileA(c->hFind, &c->cur)) {
                c->cur_ready = 1;
            } else {
                c->done = 1;
                FindClose(c->hFind);
                c->hFind = INVALID_HANDLE;
                /* cur_ready is still 0 — loop will see done=1 and break */
            }
        }
    }

    return total;
}
