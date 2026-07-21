/*===-- runtime/platform/Err.c - unhandled-error reporting ----------------===
 *
 * Part of the Fly Project https://flylang.org
 * Under the Apache License v2.0 see LICENSE for details.
 *
 * Platform-independent: reaches the OS only through io_write().
 *===----------------------------------------------------------------------===*/

#include "Runtime.h"

void err_print(i32 code, const char *msg)
{
    /* "error " + sign + 10 digits */
    char buf[24];
    usize len = 0;

    buf[len++] = 'e'; buf[len++] = 'r'; buf[len++] = 'r';
    buf[len++] = 'o'; buf[len++] = 'r'; buf[len++] = ' ';

    /* Render 'code' in decimal without libc. Negate into u32 so
     * -2147483648 does not overflow. */
    u32 mag = (code < 0) ? (u32)0 - (u32)code : (u32)code;
    if (code < 0)
        buf[len++] = '-';
    char digits[10];
    usize n = 0;
    do {
        digits[n++] = (char)('0' + (mag % 10u));
        mag /= 10u;
    } while (mag != 0u);
    while (n > 0)
        buf[len++] = digits[--n];

    io_write(STDERR, buf, len);

    if (msg && msg[0]) {
        io_write(STDERR, ": ", 2);
        usize mlen = 0;
        while (msg[mlen])
            mlen++;
        io_write(STDERR, msg, mlen);
    }
    io_write(STDERR, "\n", 1);
}
