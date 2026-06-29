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

/* ── Compiler portability ───────────────────────────────────────────────── */

#if defined(_MSC_VER)
#  define FLY_NORETURN   __declspec(noreturn)
#  define FLY_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#  define FLY_NORETURN   __attribute__((noreturn))
#  define FLY_UNREACHABLE() __builtin_unreachable()
#else
#  define FLY_NORETURN
#  define FLY_UNREACHABLE() do {} while (0)
#endif

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

/* Allocate 'size' bytes of heap memory (backed by malloc).
 * The returned pointer is compatible with free().
 * Returns NULL on failure. */
void *mem_alloc(usize size);

/* Release memory previously returned by mem_alloc (calls free).
 * 'size' is accepted for API symmetry but ignored.
 * Returns 0 on success, -1 on error. */
i32 mem_free(void *ptr, usize size);

/* Resize an allocation returned by mem_alloc to new_size bytes.
 * Semantics match realloc(): returns new pointer or NULL on failure. */
void *mem_realloc(void *ptr, usize new_size);

/* ── I/O ─────────────────────────────────────────────────────────────────── */

/* Write 'count' bytes from 'buf' to file descriptor 'fd'.
 * Returns the number of bytes written, or a negative value on error. */
i64 io_write(i32 fd, const void *buf, usize count);

/* ── Process ─────────────────────────────────────────────────────────────── */

/* Terminate all threads in the process with 'code' as the exit status.
 * Never returns. */
FLY_NORETURN void proc_exit(i32 code);

/* Spawn 'path' with the NULL-terminated 'argv', inheriting the current
 * environment, and block until it finishes. Returns the child's exit code
 * (0..255), 128+signo if it was killed by a signal, or -1 on spawn failure. */
i32 proc_exec(const char *path, char *const argv[]);

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

/* ── Filesystem ──────────────────────────────────────────────────────────── */

/* Open file at null-terminated 'path' with given 'flags' (O_RDONLY=0,
 * O_WRONLY=1, O_RDWR=2, O_CREAT=64, O_TRUNC=512, O_APPEND=1024) and
 * octal 'mode' (e.g. 0644). Returns an fd (≥ 0) on success, or -1 on error. */
i32 fs_open(const char *path, i32 flags, i32 mode);

/* Close file descriptor 'fd'. Returns 0 on success, -1 on error. */
i32 fs_close(i32 fd);

/* Read up to 'count' bytes from 'fd' into 'buf'.
 * Returns the number of bytes read (≥ 0), or -1 on error. */
i64 fs_read(i32 fd, void *buf, usize count);

/* Write 'count' bytes from 'buf' to 'fd'.
 * Returns the number of bytes written (≥ 0), or -1 on error. */
i64 fs_write(i32 fd, const void *buf, usize count);

/* Reposition 'fd'. whence: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END.
 * Returns the new file offset, or -1 on error. */
i64 fs_seek(i32 fd, i64 offset, i32 whence);

/* Stat the file at 'path'. Writes size and mode bits into the out-params.
 * Returns 0 on success, -1 on error. */
i32 fs_stat(const char *path, u64 *size_out, u32 *mode_out);

/* Single-value stat helpers — return one value each (-1 on error).
 * These are the Fly-callable wrappers (Fly only supports one non-const out-param). */
i64 fs_size(const char *path);
i32 fs_mode(const char *path);
i64 fs_lsize(const char *path);
i32 fs_lmode(const char *path);

/* Create directory 'path' with permission bits 'mode'.
 * Returns 0 on success, -1 on error. */
i32 fs_mkdir(const char *path, i32 mode);

/* Delete file at 'path'. Returns 0 on success, -1 on error. */
i32 fs_unlink(const char *path);

/* Rename 'src' to 'dst'. Returns 0 on success, -1 on error. */
i32 fs_rename(const char *src, const char *dst);

/* Stat 'path' without following symlinks. Returns 0 on success, -1 on error. */
i32 fs_lstat(const char *path, u64 *size_out, u32 *mode_out);

/* Remove empty directory 'path'. Returns 0 on success, -1 on error. */
i32 fs_rmdir(const char *path);

/* Flush kernel buffers for 'fd'. Returns 0 on success, -1 on error. */
i32 fs_fsync(i32 fd);

/* Truncate file at 'path' to 'size' bytes. Returns 0 on success, -1 on error. */
i32 fs_truncate(const char *path, i64 size);

/* Change permissions of 'path' to 'mode'. Returns 0 on success, -1 on error. */
i32 fs_chmod(const char *path, i32 mode);

/* Create a symbolic link 'link' pointing to 'target'.
 * Returns 0 on success, -1 on error. */
i32 fs_symlink(const char *target, const char *link);

/* Read the target of symlink 'path' into 'buf' (up to 'size' bytes, no '\0').
 * Returns the number of bytes placed in buf, or -1 on error. */
i32 fs_readlink(const char *path, char *buf, usize size);

/* Read directory entries from open fd into 'buf' (size bytes).
 * Returns bytes filled, 0 at end, -1 on error. (Uses getdents64 on Linux.) */
i32 fs_getdents(i32 fd, char *buf, usize size);

/* ── Time ────────────────────────────────────────────────────────────────── */

/* Get real-time clock (CLOCK_REALTIME). Writes seconds and nanoseconds into
 * the out-params. Returns 0 on success, -1 on error. */
i32 time_realtime(i64 *sec_out, i64 *nsec_out);

/* Get monotonic clock (CLOCK_MONOTONIC). Writes seconds and nanoseconds into
 * the out-params. Returns 0 on success, -1 on error. */
i32 time_monotonic(i64 *sec_out, i64 *nsec_out);

/* Single-value time helpers — Fly-callable (one non-const out-param).
 * Return nanoseconds since Unix epoch / boot, or -1 on error. */
i64 time_now_ns(void);
i64 time_mono_ns(void);

/* Sleep for 'sec' seconds and 'nsec' additional nanoseconds. */
void time_sleep(i64 sec, i64 nsec);

/* ── Environment ─────────────────────────────────────────────────────────── */

/* Copy the current working directory path (null-terminated) into 'buf'
 * (capacity 'cap' bytes). Returns the path length (without '\0') on success,
 * or -1 on error. */
i32 env_getcwd(char *buf, usize cap);

/* Change the current working directory to 'path'.
 * Returns 0 on success, -1 on error. */
i32 env_chdir(const char *path);

/* Look up environment variable 'key'. Copies value into 'buf' (up to size-1
 * bytes, null-terminated). Returns value length on success, -1 if not found. */
i32 env_get(const char *key, char *buf, usize size);

/* Set environment variable 'key' to 'val'. Returns 0 on success, -1 on error. */
i32 env_set(const char *key, const char *val);

/* Remove environment variable 'key'. Returns 0 on success, -1 on error. */
i32 env_delete(const char *key);

/* Return the number of entries in the environment. */
i32 env_all_count(void);

/* Copy the key and value of environ[idx] into the respective buffers. */
void env_all_get(i32 idx, char *key_buf, usize key_size,
                 char *val_buf, usize val_size);

/* Copy the hostname into 'buf' (up to size-1 bytes, null-terminated).
 * Returns the hostname length on success, -1 on error. */
i32 env_hostname(char *buf, usize size);

/* Return the number of command-line arguments (argc). */
i32 env_args_count(void);

/* Copy argv[idx] into 'buf' (up to size-1 bytes, null-terminated).
 * Returns the argument length on success, -1 on error. */
i32 env_args_get(i32 idx, char *buf, usize size);

/* Store argc and argv for later retrieval via env_args_count/env_args_get.
 * Called once from the fly-compiled program entry point. */
void env_init(int argc, char **argv);

/* ── Atomic operations (seq-cst / acq-rel ordering) ─────────────────────── */

/* Atomically load *addr (acquire). */
i32  atomic_load_i32(i32 *addr);

/* Atomically store val to *addr (release). */
void atomic_store_i32(i32 *addr, i32 val);

/* Compare-and-swap: if *addr == expected, write desired; return old value. */
i32  atomic_cas_i32(i32 *addr, i32 expected, i32 desired);

/* Atomically add delta to *addr; return the old value. */
i32  atomic_fetch_add_i32(i32 *addr, i32 delta);

/* ── Math (libm) ─────────────────────────────────────────────────────────── */
/* These symbols are resolved from libm (-lm on Linux, libSystem on macOS,
 * libcmt on Windows) at Fly program link time.  They are NOT implemented in
 * the runtime C sources — these declarations exist so C test code in this
 * tree can call them without pulling in <math.h>. */

double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);
double sinh(double x);
double cosh(double x);
double tanh(double x);
double asinh(double x);
double acosh(double x);
double atanh(double x);
double erf(double x);
double erfc(double x);
double tgamma(double x);
double hypot(double x, double y);

/* ── Fly string-array helpers ────────────────────────────────────────────── */

/* Write a Fly string struct { i64 ptr, i32 size } at arr_base[idx * 16]. */
void str_slot_set(char *arr_base, i32 idx, char *ptr, i32 size);

/* Read a Fly string struct from arr_base[idx * 16]. */
void str_slot_get(char *arr_base, i32 idx, i64 *ptr_out, i32 *size_out);

/* Single-value string-slot helpers — Fly-callable (one non-const out-param). */
i64 str_slot_ptr(char *arr_base, i32 idx);
i32 str_slot_size(char *arr_base, i32 idx);

/* (Note: string/numeric value primitives like strMake/strByteAt/ulongToLong are
 * NOT here — fly.runtime can't reliably pass Fly value types across the C ABI;
 * those are Fly wrappers over fly.llvm in fly.str / fly.mem.) */

/* Fill arr_base with 'count' Fly string structs from argv[].
 * Each string is a fresh mem_alloc'd copy of the argument. */
void env_args_fill(char *arr_base, i32 count);

/* Fill arr_base with 'count' Fly string structs from environ[] (KEY=VALUE). */
void env_all_fill(char *arr_base, i32 count);

/* Copy the OS name ("linux", "macos", "windows") into buf.
 * Returns the length on success, -1 on error. */
i32 env_osname(char *buf, usize size);

/* Host platform details from uname(2): CPU architecture ("x86_64"), kernel
 * release ("7.0.0-22-generic"), kernel version (build string). Copy into buf;
 * return the length on success, -1 on error. */
i32 env_arch(char *buf, usize size);
i32 env_kernel(char *buf, usize size);
i32 env_kernelversion(char *buf, usize size);

/* Absolute path of the running executable (Linux: readlink /proc/self/exe;
 * macOS: _NSGetExecutablePath; Windows: GetModuleFileNameW). Independent of
 * argv[0]. Copy into buf; return the length on success, -1 on error. */
i32 env_exe_path(char *buf, usize size);

#endif /* FLY_RUNTIME_H */
