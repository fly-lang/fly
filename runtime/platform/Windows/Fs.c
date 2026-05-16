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

i32 fs_open(const char *path, i32 flags, i32 mode)
{
    int r = _open(path, posix_flags_to_win(flags), (int)mode);
    return (r >= 0) ? (i32)r : -1;
}

i32 fs_close(i32 fd)
{
    return (_close((int)fd) == 0) ? 0 : -1;
}

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

i64 fs_lsize(const char *path) { return fs_size(path); }
i32 fs_lmode(const char *path) { return fs_mode(path); }

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

i32 fs_lstat(const char *path, u64 *size_out, u32 *mode_out)
{
    /* Windows doesn't distinguish lstat from stat for regular files */
    return fs_stat(path, size_out, mode_out);
}

i32 fs_rmdir(const char *path)
{
    return RemoveDirectoryA(path) ? 0 : -1;
}

i32 fs_fsync(i32 fd)
{
    /* _get_osfhandle converts CRT fd to Win32 HANDLE */
    (void)fd;
    return 0; /* no-op: not easily done without _get_osfhandle */
}

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

i32 fs_symlink(const char *target, const char *link)
{
    return CreateSymbolicLinkA(link, target, 0) ? 0 : -1;
}

i32 fs_readlink(const char *path, char *buf, usize size)
{
    (void)path; (void)buf; (void)size;
    return -1; /* not implemented on Windows in this minimal runtime */
}

i32 fs_getdents(i32 fd, char *buf, usize size)
{
    (void)fd; (void)buf; (void)size;
    return -1; /* not implemented on Windows in this minimal runtime */
}
