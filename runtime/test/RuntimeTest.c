/*===-- test/RuntimeTest.c - Cross-platform runtime smoke test ------------===
 *
 * Exercises all four runtime primitives without libc.
 *
 * Entry point:
 *   Linux   — _start  (freestanding, no CRT)
 *   macOS   — main    (linked against -lSystem which provides crt1.o)
 *   Windows — main    (CRT provides mainCRTStartup)
 *
 * Expected stdout:
 *   fly runtime ok
 *
 * Exit code: 0 on success, 1 on any failure.
 *===----------------------------------------------------------------------===*/

#include "../platform/Runtime.h"

static usize str_len(const char *s)
{
    usize n = 0;
    while (s[n]) n++;
    return n;
}

static void print(const char *msg) { io_write(STDOUT, msg, str_len(msg)); }
static void fail(const char *msg)
{
    io_write(STDERR, msg, str_len(msg));
    proc_exit(1);
}

/* ── thread test state ──────────────────────────────────────────────────── */

static volatile i32 thread_done = 0;

static void thread_fn(void *arg)
{
    print((const char *)arg);
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" ::: "memory");
#endif
    thread_done = 1;
    futex_wake((i32 *)&thread_done, 1);
}

/* ── math smoke test ────────────────────────────────────────────────────── */

static void test_math(void)
{
    if (asin(0.0)    != 0.0) proc_exit(10);
    if (acos(1.0)    != 0.0) proc_exit(11);
    if (atan(0.0)    != 0.0) proc_exit(12);
    if (atan2(0.0, 1.0) != 0.0) proc_exit(13);
    if (sinh(0.0)    != 0.0) proc_exit(14);
    if (cosh(0.0)    != 1.0) proc_exit(15);
    if (tanh(0.0)    != 0.0) proc_exit(16);
    if (asinh(0.0)   != 0.0) proc_exit(17);
    if (acosh(1.0)   != 0.0) proc_exit(18);
    if (atanh(0.0)   != 0.0) proc_exit(19);
    if (erf(0.0)     != 0.0) proc_exit(20);
    if (erfc(0.0)    != 1.0) proc_exit(21);
    if (tgamma(1.0)  != 1.0) proc_exit(22);
    if (hypot(3.0, 4.0) != 5.0) proc_exit(23);
}

/* ── process-exec test (platform-gated executables) ─────────────────────── */

/* Spawn a known external command and assert its exit code. Uses paths that
 * exist on the host OS the runtime is being built for:
 *   Linux  — /bin/true (0), /bin/false (1)
 *   macOS  — /usr/bin/true (0), /usr/bin/false (1)
 *   Windows — cmd.exe /c exit N
 * Skipped where no portable command is guaranteed. */
static void test_proc(void)
{
#if defined(__linux__)
    const char *true_path  = "/bin/true";
    const char *false_path = "/bin/false";
#endif
#if defined(__APPLE__)
    const char *true_path  = "/usr/bin/true";
    const char *false_path = "/usr/bin/false";
#endif
#if defined(__linux__) || defined(__APPLE__)
    char *const true_argv[]  = { (char *)true_path,  0 };
    char *const false_argv[] = { (char *)false_path, 0 };
    if (proc_exec(true_path, true_argv) != 0)
        fail("FAIL: proc_exec(true) did not return 0\n");
    if (proc_exec(false_path, false_argv) != 1)
        fail("FAIL: proc_exec(false) did not return 1\n");
#elif defined(_WIN32)
    const char *cmd = "C:\\Windows\\System32\\cmd.exe";
    char *const ok_argv[]  = { (char *)"cmd", (char *)"/c", (char *)"exit 0", 0 };
    char *const err_argv[] = { (char *)"cmd", (char *)"/c", (char *)"exit 1", 0 };
    if (proc_exec(cmd, ok_argv) != 0)
        fail("FAIL: proc_exec(cmd exit 0) did not return 0\n");
    if (proc_exec(cmd, err_argv) != 1)
        fail("FAIL: proc_exec(cmd exit 1) did not return 1\n");
#endif
}

/* ── tests ──────────────────────────────────────────────────────────────── */

static void run_tests(void)
{
    /* 0. Math (libm linkage smoke test) */
    test_math();

    /* 1. Memory */
    const usize PAGE = 4096;
    void *mem = mem_alloc(PAGE);
    if (!mem) fail("FAIL: mem_alloc returned NULL\n");
    volatile u8 *p = (volatile u8 *)mem;
    p[0] = 0xAB;
    if (p[0] != 0xAB) fail("FAIL: memory read-back mismatch\n");
    if (mem_free(mem, PAGE) != 0) fail("FAIL: mem_free failed\n");

    /* 2. I/O */
    i64 written = io_write(STDOUT, "fly runtime ok\n", 15);
    if (written != 15) fail("FAIL: io_write returned unexpected count\n");

    /* 2b. Process exec (spawn an external command, check exit code) */
    test_proc();

    /* 3. Thread + futex */
    tid t = thread_spawn(thread_fn, (void *)"thread: hello from thread\n", 65536);
    if (t <= 0) fail("FAIL: thread_spawn failed\n");
    while (thread_done == 0)
        futex_wait((i32 *)&thread_done, 0);

    /* 4. Exit */
    proc_exit(0);
}

/* ── platform entry points ──────────────────────────────────────────────── */

#if defined(__linux__)

void _start(void) __attribute__((noreturn));
void _start(void) { run_tests(); __builtin_unreachable(); }

#else

int main(void) { run_tests(); return 0; }

#endif
