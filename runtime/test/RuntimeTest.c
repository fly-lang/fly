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

/* ── tests ──────────────────────────────────────────────────────────────── */

static void run_tests(void)
{
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
