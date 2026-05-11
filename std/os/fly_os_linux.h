/*===-- std/os/fly_os_linux.h - Linux x86-64 syscall layer (private) ----===
 *
 * Private header — included only by fly_os_*.c, never installed.
 * Defines Linux syscall numbers, kernel struct layouts, open/stat/clock
 * constants, and lightweight inline __os_syscall helpers.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_LINUX_H
#define FLY_OS_LINUX_H

/* ── Syscall numbers (x86-64) ────────────────────────────────────────────── */

#define SYS_read          0
#define SYS_write         1
#define SYS_open          2
#define SYS_close         3
#define SYS_stat          4
#define SYS_fstat         5
#define SYS_lstat         6
#define SYS_lseek         8
#define SYS_truncate      76
#define SYS_ftruncate     77
#define SYS_fsync         74
#define SYS_getcwd        79
#define SYS_chdir         80
#define SYS_rename        82
#define SYS_mkdir         83
#define SYS_rmdir         84
#define SYS_link          86
#define SYS_unlink        87
#define SYS_symlink       88
#define SYS_readlink      89
#define SYS_chmod         90
#define SYS_uname         63
#define SYS_nanosleep     35
#define SYS_getdents64    217
#define SYS_clock_gettime 228
#define SYS_exit_group    231
#define SYS_pipe2         293

/* ── open(2) flags ───────────────────────────────────────────────────────── */

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_CREAT     64      /* 0100 */
#define O_EXCL      128     /* 0200 */
#define O_TRUNC     512     /* 01000 */
#define O_APPEND    1024    /* 02000 */
#define O_DIRECTORY 65536   /* 0200000 */
#define O_CLOEXEC   524288  /* 02000000 */
#define O_NONBLOCK  2048    /* 04000 */

/* ── stat(2) mode bits ───────────────────────────────────────────────────── */

#define S_IFMT    0170000
#define S_IFREG   0100000
#define S_IFDIR   0040000
#define S_IFLNK   0120000

#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)

/* ── clock_gettime(2) IDs ────────────────────────────────────────────────── */

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

/* ── getdents64 d_type values ────────────────────────────────────────────── */

#define DT_DIR  4
#define DT_REG  8
#define DT_LNK  10

/* ── Kernel struct layouts (x86-64) ─────────────────────────────────────── */

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
} linux_stat;   /* 144 bytes */

typedef struct {
    unsigned long long d_ino;
    long long          d_off;
    unsigned short     d_reclen;
    unsigned char      d_type;
    char               d_name[1]; /* variable — accessed via buffer + cast */
} linux_dirent64;

typedef struct {
    long tv_sec;
    long tv_nsec;
} linux_timespec;

typedef struct {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
} linux_utsname;

/* ── Inline syscall helpers (x86-64 ABI) ────────────────────────────────── */

static __inline__ long __os_sc1(long n, long a1) {
    long r;
    __asm__ __volatile__("syscall"
        : "=a"(r) : "0"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}
static __inline__ long __os_sc2(long n, long a1, long a2) {
    long r;
    __asm__ __volatile__("syscall"
        : "=a"(r) : "0"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
    return r;
}
static __inline__ long __os_sc3(long n, long a1, long a2, long a3) {
    long r;
    __asm__ __volatile__("syscall"
        : "=a"(r) : "0"(n), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
    return r;
}
static __inline__ long __os_sc4(long n, long a1, long a2, long a3, long a4) {
    long r;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__("syscall"
        : "=a"(r) : "0"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "rcx", "r11", "memory");
    return r;
}

#endif /* FLY_OS_LINUX_H */
