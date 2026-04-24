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

#endif /* FLY_RUNTIME_MACOS_LIBSYSTEM_H */
