/*===-- src/Runtime/macOS/LibSystem.h - Minimal libSystem declarations ----===
 *
 * Private header — not installed, used only by src/Runtime/macOS/ sources.
 *
 * macOS-specific notes:
 *   MAP_ANON = 0x1000  (Linux uses 0x20 / MAP_ANONYMOUS)
 *   __ulock_wait / __ulock_wake are private XNU syscalls in libSystem.B,
 *     available on macOS 10.12+.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_RUNTIME_MACOS_LIBSYSTEM_H
#define FLY_RUNTIME_MACOS_LIBSYSTEM_H

typedef long           ssize_rt;
typedef unsigned long  size_rt;

/* ── mmap / munmap ──────────────────────────────────────────────────────── */

#define PROT_READ   0x01
#define PROT_WRITE  0x02
#define MAP_PRIVATE 0x0002
#define MAP_ANON    0x1000
#define MAP_FAILED  ((void *)-1)

extern void  *mmap(void *addr, size_rt len, int prot, int flags,
                   int fd, long long offset);
extern int    munmap(void *addr, size_rt len);

/* ── write / _exit ──────────────────────────────────────────────────────── */

extern ssize_rt write(int fd, const void *buf, size_rt count);
extern void     _exit(int status) __attribute__((noreturn));

/* ── pthread ────────────────────────────────────────────────────────────── */

typedef void *pthread_t_rt;
typedef struct { char __opaque[64]; } pthread_attr_t_rt;
typedef void *(*pthread_start_fn)(void *);

extern int pthread_attr_init(pthread_attr_t_rt *attr);
extern int pthread_attr_setstacksize(pthread_attr_t_rt *attr, size_rt stacksize);
extern int pthread_attr_destroy(pthread_attr_t_rt *attr);
extern int pthread_create(pthread_t_rt *thread, const pthread_attr_t_rt *attr,
                          pthread_start_fn start_routine, void *arg);

/* ── __ulock_wait / __ulock_wake (XNU private, macOS 10.12+) ────────────── */
/*
 * int __ulock_wait(uint32_t op, void *addr, uint64_t value, uint32_t timeout_us)
 * int __ulock_wake(uint32_t op, void *addr, uint64_t wake_value)
 */
#define UL_COMPARE_AND_WAIT  1u
#define ULF_WAKE_ALL         0x00000100u

extern int __ulock_wait(unsigned int op, void *addr,
                        unsigned long long value, unsigned int timeout_us);
extern int __ulock_wake(unsigned int op, void *addr,
                        unsigned long long wake_value);

/* ── Filesystem ─────────────────────────────────────────────────────────── */

#define O_RDONLY   0x0000
#define O_WRONLY   0x0001
#define O_RDWR     0x0002
#define O_CREAT    0x0200
#define O_TRUNC    0x0400
#define O_APPEND   0x0008

extern int     open(const char *path, int flags, int mode);
extern int     close(int fd);
extern ssize_rt read(int fd, void *buf, size_rt count);
extern long long lseek(int fd, long long offset, int whence);
extern int     stat(const char *path, void *buf);
extern int     lstat(const char *path, void *buf);
extern int     mkdir(const char *path, unsigned int mode);
extern int     rmdir(const char *path);
extern int     unlink(const char *path);
extern int     rename(const char *src, const char *dst);
extern int     fsync(int fd);
extern int     truncate(const char *path, long long length);
extern int     chmod(const char *path, unsigned int mode);
extern int     symlink(const char *target, const char *link);
extern long    readlink(const char *path, char *buf, size_rt bufsz);
extern long    getdirentries64(int fd, void *buf, size_rt bufcount, long long *basep);

/* ── Time ───────────────────────────────────────────────────────────────── */

typedef struct { long long tv_sec; long tv_nsec; } macos_timespec_t;

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 6

extern int clock_gettime(int clk_id, macos_timespec_t *ts);
extern int nanosleep(const macos_timespec_t *req, macos_timespec_t *rem);

/* ── Environment ────────────────────────────────────────────────────────── */

extern char *getcwd(char *buf, size_rt size);
extern int   chdir(const char *path);
extern char *getenv(const char *name);
extern int   setenv(const char *name, const char *value, int overwrite);
extern int   unsetenv(const char *name);
extern char **environ;
extern int   gethostname(char *name, size_rt len);
extern size_rt strlen(const char *s);
extern char  *strchr(const char *s, int c);

#endif /* FLY_RUNTIME_MACOS_LIBSYSTEM_H */
