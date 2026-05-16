/*===-- runtime/Linux/Fs.c - Filesystem primitives via Linux syscalls ----===*/

#include "../Runtime.h"
#include "Syscall.h"

/* ── open(2) flags ───────────────────────────────────────────────────────── */

#define O_RDONLY   0
#define O_WRONLY   1
#define O_RDWR     2
#define O_CREAT    64
#define O_TRUNC    512
#define O_APPEND   1024

/* ── Minimal stat layout (x86-64) ───────────────────────────────────────── */

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

/* ── timespec for nanosleep ─────────────────────────────────────────────── */

typedef struct { long tv_sec; long tv_nsec; } linux_timespec_t;

/* ═══════════════════════════════════════════════════════════════════════════ */

i32 fs_open(const char *path, i32 flags, i32 mode)
{
    long r = __syscall3(SYS_open, (long)path, (long)flags, (long)mode);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_close(i32 fd)
{
    long r = __syscall1(SYS_close, (long)fd);
    return (r == 0) ? 0 : -1;
}

i64 fs_read(i32 fd, void *buf, usize count)
{
    long r = __syscall3(SYS_read, (long)fd, (long)buf, (long)count);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_write(i32 fd, const void *buf, usize count)
{
    long r = __syscall3(SYS_write, (long)fd, (long)buf, (long)count);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_seek(i32 fd, i64 offset, i32 whence)
{
    long r = __syscall3(SYS_lseek, (long)fd, (long)offset, (long)whence);
    return (r >= 0) ? (i64)r : -1;
}

i32 fs_stat(const char *path, u64 *size_out, u32 *mode_out)
{
    linux_stat_t st;
    long r = __syscall2(SYS_stat, (long)path, (long)&st);
    if (r < 0) {
        *size_out = 0;
        *mode_out = 0;
        return -1;
    }
    *size_out = (u64)st.st_size;
    *mode_out = (u32)st.st_mode;
    return 0;
}

/* Single-value stat helpers (Fly-compatible: one return value each) */
i64 fs_size(const char *path)
{
    linux_stat_t st;
    long r = __syscall2(SYS_stat, (long)path, (long)&st);
    return (r < 0) ? -1 : (i64)st.st_size;
}

i32 fs_mode(const char *path)
{
    linux_stat_t st;
    long r = __syscall2(SYS_stat, (long)path, (long)&st);
    return (r < 0) ? -1 : (i32)st.st_mode;
}

i64 fs_lsize(const char *path)
{
    linux_stat_t st;
    long r = __syscall2(SYS_lstat, (long)path, (long)&st);
    return (r < 0) ? -1 : (i64)st.st_size;
}

i32 fs_lmode(const char *path)
{
    linux_stat_t st;
    long r = __syscall2(SYS_lstat, (long)path, (long)&st);
    return (r < 0) ? -1 : (i32)st.st_mode;
}

i32 fs_mkdir(const char *path, i32 mode)
{
    long r = __syscall2(SYS_mkdir, (long)path, (long)mode);
    return (r == 0) ? 0 : -1;
}

i32 fs_unlink(const char *path)
{
    long r = __syscall1(SYS_unlink, (long)path);
    return (r == 0) ? 0 : -1;
}

i32 fs_rename(const char *src, const char *dst)
{
    long r = __syscall2(SYS_rename, (long)src, (long)dst);
    return (r == 0) ? 0 : -1;
}

i32 fs_lstat(const char *path, u64 *size_out, u32 *mode_out)
{
    linux_stat_t st;
    long r = __syscall2(SYS_lstat, (long)path, (long)&st);
    if (r < 0) {
        *size_out = 0;
        *mode_out = 0;
        return -1;
    }
    *size_out = (u64)st.st_size;
    *mode_out = (u32)st.st_mode;
    return 0;
}

i32 fs_rmdir(const char *path)
{
    long r = __syscall1(SYS_rmdir, (long)path);
    return (r == 0) ? 0 : -1;
}

i32 fs_fsync(i32 fd)
{
    long r = __syscall1(SYS_fsync, (long)fd);
    return (r == 0) ? 0 : -1;
}

i32 fs_truncate(const char *path, i64 size)
{
    long r = __syscall2(SYS_truncate, (long)path, (long)size);
    return (r == 0) ? 0 : -1;
}

i32 fs_chmod(const char *path, i32 mode)
{
    long r = __syscall2(SYS_chmod, (long)path, (long)mode);
    return (r == 0) ? 0 : -1;
}

i32 fs_symlink(const char *target, const char *link)
{
    long r = __syscall2(SYS_symlink, (long)target, (long)link);
    return (r == 0) ? 0 : -1;
}

i32 fs_readlink(const char *path, char *buf, usize size)
{
    long r = __syscall3(SYS_readlink, (long)path, (long)buf, (long)size);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_getdents(i32 fd, char *buf, usize size)
{
    long r = __syscall3(SYS_getdents64, (long)fd, (long)buf, (long)size);
    return (r >= 0) ? (i32)r : -1;
}
