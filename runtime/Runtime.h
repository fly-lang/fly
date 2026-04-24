/*===-- include/Runtime/Runtime.h - Fly minimal runtime public API ---------===
 *
 * Part of the Fly Project https://flylang.org
 * Under the Apache License v2.0 see LICENSE for details.
 *
 * Freestanding header: no libc, no system headers.
 * All Fly-compiled programs link against this runtime.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_RUNTIME_H
#define FLY_RUNTIME_H

/* ── Primitive types (no stdint.h / no libc headers) ───────────────────── */

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef u64                usize;   /* pointer-width unsigned on all LP64 targets */
typedef i64                tid;     /* thread id */

/* ── Well-known file descriptors ────────────────────────────────────────── */

/* POSIX defines STDIN_FILENO / STDOUT_FILENO / STDERR_FILENO; the short forms
 * below do not clash with any standard macro. */
#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* ── Memory ─────────────────────────────────────────────────────────────── */

/* Allocate 'size' bytes of anonymous read/write memory (page-granularity).
 * Returns NULL on failure. */
void *mem_alloc(usize size);

/* Release a mapping previously returned by mem_alloc.
 * Returns 0 on success, -1 on error. */
i32 mem_free(void *ptr, usize size);

/* ── I/O ─────────────────────────────────────────────────────────────────── */

/* Write 'count' bytes from 'buf' to file descriptor 'fd'.
 * Returns the number of bytes written, or a negative value on error. */
i64 io_write(i32 fd, const void *buf, usize count);

/* ── Process ─────────────────────────────────────────────────────────────── */

/* Terminate all threads in the process with 'code' as the exit status.
 * Never returns. */
__attribute__((noreturn)) void proc_exit(i32 code);

/* ── Threads ─────────────────────────────────────────────────────────────── */

/* Spawn a new thread that executes fn(arg).
 * stack_size bytes are allocated internally.
 * Returns the TID (> 0) on success, or a negative value on error. */
tid thread_spawn(void (*fn)(void *), void *arg, usize stack_size);

/* Block until *addr != expected.
 * Returns 0 on wake, -1 on error. */
i32 futex_wait(i32 *addr, i32 expected);

/* Wake at most 'count' threads waiting on *addr.
 * Returns the number of threads woken, or -1 on error. */
i32 futex_wake(i32 *addr, i32 count);

#endif /* FLY_RUNTIME_H */
