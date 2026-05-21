/*===-- runtime/macOS/Fs.c - Filesystem primitives via libSystem ---------===*/

#include "../Runtime.h"
#include "LibSystem.h"

/* Minimal macOS (arm64/x86-64) stat layout — only fields we need. */
typedef struct {
    int            st_dev;
    unsigned short st_mode;
    unsigned short st_nlink;
    unsigned long long st_ino;
    unsigned int   st_uid;
    unsigned int   st_gid;
    int            st_rdev;
    /* timespec * 3: atime, mtime, ctime (each = {long, long}) */
    long long      st_atime_sec;  long st_atime_nsec;
    long long      st_mtime_sec;  long st_mtime_nsec;
    long long      st_ctime_sec;  long st_ctime_nsec;
    long long      st_birthtime_sec; long st_birthtime_nsec;
    long long      st_size;
    long long      st_blocks;
    int            st_blksize;
    unsigned int   st_flags;
    unsigned int   st_gen;
    int            st_lspare;
    long long      st_qspare[2];
} macos_stat_t;

i32 fs_open(const char *path, i32 flags, i32 mode)
{
    int r = open(path, (int)flags, (int)mode);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_close(i32 fd)
{
    return (close((int)fd) == 0) ? 0 : -1;
}

i64 fs_read(i32 fd, void *buf, usize count)
{
    ssize_rt r = read((int)fd, buf, (size_rt)count);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_write(i32 fd, const void *buf, usize count)
{
    ssize_rt r = write((int)fd, buf, (size_rt)count);
    return (r >= 0) ? (i64)r : -1;
}

i64 fs_seek(i32 fd, i64 offset, i32 whence)
{
    long long r = lseek((int)fd, (long long)offset, (int)whence);
    return (r >= 0) ? (i64)r : -1;
}

i32 fs_stat(const char *path, u64 *size_out, u32 *mode_out)
{
    macos_stat_t st;
    int r = stat(path, &st);
    if (r != 0) { *size_out = 0; *mode_out = 0; return -1; }
    *size_out = (u64)st.st_size;
    *mode_out = (u32)st.st_mode;
    return 0;
}

/* Single-value stat helpers (Fly-compatible: one return value each) */
i64 fs_size(const char *path)
{
    macos_stat_t st;
    return (stat(path, &st) == 0) ? (i64)st.st_size : -1;
}

i32 fs_mode(const char *path)
{
    macos_stat_t st;
    return (stat(path, &st) == 0) ? (i32)st.st_mode : -1;
}

i64 fs_lsize(const char *path)
{
    macos_stat_t st;
    return (lstat(path, &st) == 0) ? (i64)st.st_size : -1;
}

i32 fs_lmode(const char *path)
{
    macos_stat_t st;
    return (lstat(path, &st) == 0) ? (i32)st.st_mode : -1;
}

i32 fs_mkdir(const char *path, i32 mode)
{
    return (mkdir(path, (unsigned int)mode) == 0) ? 0 : -1;
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
    macos_stat_t st;
    int r = lstat(path, &st);
    if (r != 0) { *size_out = 0; *mode_out = 0; return -1; }
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
    return (fsync((int)fd) == 0) ? 0 : -1;
}

i32 fs_truncate(const char *path, i64 size)
{
    return (truncate(path, (long long)size) == 0) ? 0 : -1;
}

i32 fs_chmod(const char *path, i32 mode)
{
    return (chmod(path, (unsigned int)mode) == 0) ? 0 : -1;
}

i32 fs_symlink(const char *target, const char *link)
{
    return (symlink(target, link) == 0) ? 0 : -1;
}

i32 fs_readlink(const char *path, char *buf, usize size)
{
    long r = readlink(path, buf, (size_rt)size);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_getdents(i32 fd, char *buf, usize size)
{
    long long base = 0;
    long r = getdirentries64((int)fd, buf, (size_rt)size, &base);
    return (r >= 0) ? (i32)r : -1;
}
