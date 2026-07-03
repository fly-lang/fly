/*===-- runtime/Linux/Fs.c - Filesystem primitives via libc -------------===*/

#include "../Runtime.h"

/* ── open(2) flags ───────────────────────────────────────────────────────── */

#define O_RDONLY   0
#define O_WRONLY   1
#define O_RDWR     2
#define O_CREAT    64
#define O_TRUNC    512
#define O_APPEND   1024

/* ── Minimal stat layout (x86-64) — matches glibc `struct stat` field offsets
 * we read: st_mode @ 24, st_size @ 48. ─────────────────────────────────────── */

typedef struct {
    unsigned long  st_dev;
    unsigned long  st_ino;
    unsigned long  st_nlink;
    unsigned int   st_mode;
    unsigned int   st_uid;
    unsigned int   st_gid;
    unsigned int   __pad0;
    unsigned long  st_rdev;
    long           st_size;
    long           st_blksize;
    long           st_blocks;
    unsigned long  st_atime;
    unsigned long  st_atime_nsec;
    unsigned long  st_mtime;
    unsigned long  st_mtime_nsec;
    unsigned long  st_ctime;
    unsigned long  st_ctime_nsec;
    long           __unused[3];
} linux_stat_t;

/* ── libc (forward-declared under -nostdinc) ───────────────────────────────── */
extern i32 open(const char *path, i32 flags, u32 mode);
extern i32 close(i32 fd);
extern i64 read(i32 fd, void *buf, usize count);
extern i64 write(i32 fd, const void *buf, usize count);
extern i64 lseek(i32 fd, i64 offset, i32 whence);
extern i32 stat(const char *path, void *statbuf);
extern i32 lstat(const char *path, void *statbuf);
extern i32 mkdir(const char *path, u32 mode);
extern i32 unlink(const char *path);
extern i32 rename(const char *oldp, const char *newp);
extern i32 rmdir(const char *path);
extern i32 fsync(i32 fd);
extern i32 truncate(const char *path, i64 length);
extern i32 chmod(const char *path, u32 mode);
extern i32 symlink(const char *target, const char *linkpath);
extern i64 readlink(const char *path, char *buf, usize bufsiz);
extern i64 getdents64(i32 fd, void *dirp, usize count); /* glibc >= 2.30 */

/* ═══════════════════════════════════════════════════════════════════════════ */

i32 fs_open(const char *path, i32 flags, i32 mode)
{
    i32 r = open(path, flags, (u32)mode);
    return (r >= 0) ? r : -1;
}

i32 fs_close(i32 fd)
{
    return (close(fd) == 0) ? 0 : -1;
}

i64 fs_read(i32 fd, void *buf, usize count)
{
    i64 r = read(fd, buf, count);
    return (r >= 0) ? r : -1;
}

i64 fs_write(i32 fd, const void *buf, usize count)
{
    i64 r = write(fd, buf, count);
    return (r >= 0) ? r : -1;
}

i64 fs_seek(i32 fd, i64 offset, i32 whence)
{
    i64 r = lseek(fd, offset, whence);
    return (r >= 0) ? r : -1;
}

i32 fs_stat(const char *path, u64 *size_out, u32 *mode_out)
{
    linux_stat_t st;
    if (stat(path, &st) != 0) { *size_out = 0; *mode_out = 0; return -1; }
    *size_out = (u64)st.st_size;
    *mode_out = (u32)st.st_mode;
    return 0;
}

/* Single-value stat helpers (Fly-compatible: one return value each) */
i64 fs_size(const char *path)
{
    linux_stat_t st;
    return (stat(path, &st) == 0) ? (i64)st.st_size : -1;
}

i32 fs_mode(const char *path)
{
    linux_stat_t st;
    return (stat(path, &st) == 0) ? (i32)st.st_mode : -1;
}

i64 fs_lsize(const char *path)
{
    linux_stat_t st;
    return (lstat(path, &st) == 0) ? (i64)st.st_size : -1;
}

i32 fs_lmode(const char *path)
{
    linux_stat_t st;
    return (lstat(path, &st) == 0) ? (i32)st.st_mode : -1;
}

i32 fs_mkdir(const char *path, i32 mode)
{
    return (mkdir(path, (u32)mode) == 0) ? 0 : -1;
}

i32 fs_unlink(const char *path)
{
    return (unlink(path) == 0) ? 0 : -1;
}

i32 fs_rename(const char *src, const char *dst)
{
    return (rename(src, dst) == 0) ? 0 : -1;
}

i32 fs_lstat(const char *path, u64 *size_out, u32 *mode_out)
{
    linux_stat_t st;
    if (lstat(path, &st) != 0) { *size_out = 0; *mode_out = 0; return -1; }
    *size_out = (u64)st.st_size;
    *mode_out = (u32)st.st_mode;
    return 0;
}

i32 fs_rmdir(const char *path)
{
    return (rmdir(path) == 0) ? 0 : -1;
}

i32 fs_fsync(i32 fd)
{
    return (fsync(fd) == 0) ? 0 : -1;
}

i32 fs_truncate(const char *path, i64 size)
{
    return (truncate(path, size) == 0) ? 0 : -1;
}

i32 fs_chmod(const char *path, i32 mode)
{
    return (chmod(path, (u32)mode) == 0) ? 0 : -1;
}

i32 fs_symlink(const char *target, const char *link)
{
    return (symlink(target, link) == 0) ? 0 : -1;
}

i32 fs_readlink(const char *path, char *buf, usize size)
{
    i64 r = readlink(path, buf, size);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_getdents(i32 fd, char *buf, usize size)
{
    i64 r = getdents64(fd, buf, size);
    return (r >= 0) ? (i32)r : -1;
}
