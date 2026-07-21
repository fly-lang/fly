/*===-- runtime/platform/Suite.c - suite runner report helpers ------------===
 *
 * Part of the Fly Project https://flylang.org
 * Under the Apache License v2.0 see LICENSE for details.
 *
 * Called by the compiler-generated implicit main() of a `suite` binary.
 * Platform-independent: reaches the OS only through io_write().
 *===----------------------------------------------------------------------===*/

#include "Runtime.h"

static void wr(const char *s)
{
    usize n = 0;
    while (s[n])
        n++;
    io_write(STDOUT, s, n);
}

static void wr_dec(i32 v)
{
    char buf[12];
    usize len = 0;
    u32 mag = (v < 0) ? (u32)0 - (u32)v : (u32)v;
    if (v < 0)
        buf[len++] = '-';
    char digits[10];
    usize n = 0;
    do {
        digits[n++] = (char)('0' + (mag % 10u));
        mag /= 10u;
    } while (mag != 0u);
    while (n > 0)
        buf[len++] = digits[--n];
    io_write(STDOUT, buf, len);
}

static void wr_fail(i32 code, const char *msg)
{
    wr(" FAIL(");
    wr_dec(code);
    wr(")");
    if (msg && msg[0]) {
        wr(": ");
        wr(msg);
    }
    wr("\n");
}

void suite_begin(const char *name)
{
    wr("suite ");
    wr(name);
    wr("\n");
}

void suite_method(const char *name)
{
    wr("  ");
    wr(name);
    wr("\n");
}

void suite_case_begin(const char *label)
{
    /* No newline: the case's result (or a crash) completes the line, so the
     * last printed text always names the case that was running. */
    wr("    ");
    wr(label);
    wr(" ...");
}

void suite_case_result(i32 code, const char *msg)
{
    if (code == 0) {
        wr(" ok\n");
        return;
    }
    wr_fail(code, msg);
}

void suite_step_fail(const char *label, i32 code, const char *msg)
{
    wr("    ");
    wr(label);
    wr(" ...");
    wr_fail(code, msg);
}

void suite_end(const char *name, i32 total, i32 failed)
{
    wr("suite ");
    wr(name);
    wr(": ");
    wr_dec(total);
    wr(" cases, ");
    wr_dec(total - failed);
    wr(" passed, ");
    wr_dec(failed);
    wr(" failed\n");
}
