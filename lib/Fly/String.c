/*===-- lib/Fly/String.c - fly.string native implementations -----------===
 *
 * Functions in this file implement fly.string primitives that require
 * operations not yet expressible in pure Fly source (array subscript
 * write, unary minus, heap allocation).  They are compiled by the host C
 * compiler and linked into the same archive as the Fly-compiled objects.
 *
 * Calling convention (all Fly functions, non-main):
 *   arg 0 : void *err_ctx   — implicit error-handler pointer (ptr %0 in IR)
 *   arg 1 : T *param        — first explicit parameter, passed by reference
 *   arg N : T *param        — Nth explicit parameter, passed by reference
 *   const params carry the LLVM 'readonly' attribute on the pointer.
 *
 * Name mangling:
 *   _F  <len>  <name>  <param-type-codes...>
 *   int    → _i     string → _Ss
 *   so  convert(const int src, string out)  →  _F7convert_i_Ss
 *===----------------------------------------------------------------------===*/

#include "Runtime.h"

/* ── convert ────────────────────────────────────────────────────────────── */

/* Fly declaration (for reference):
 *   public convert(const int src, string out)
 *
 * Converts the 32-bit signed integer *src to its decimal ASCII representation
 * and stores a heap-allocated, null-terminated string into *out.
 *
 * The caller owns the returned string; it must be freed with mem_free when no
 * longer needed.  String length is at most 11 bytes ("-2147483648") + '\0'.
 */
void _F7convert_i_Ss(void *err_ctx, const int *src, char **out)
{
    (void)err_ctx;  /* error handling not yet used by this function */

    int n = *src;

    /* Special case: zero */
    if (n == 0) {
        char *s = (char *)mem_alloc(2);
        if (!s) return;
        s[0] = '0';
        s[1] = '\0';
        *out = s;
        return;
    }

    /* Build digits in a stack buffer (max 12 bytes for any i32). */
    char buf[12];
    int  len  = 0;
    int  neg  = (n < 0);

    /* Work on absolute value.
     * Special case INT_MIN (-2147483648): negating it overflows.
     * Handle by extracting the last digit before negating. */
    unsigned int un;
    if (neg) {
        /* Cast to unsigned first to avoid UB on INT_MIN. */
        un = (unsigned int)(0 - (unsigned int)n);
    } else {
        un = (unsigned int)n;
    }

    while (un > 0) {
        buf[len++] = (char)('0' + (un % 10));
        un /= 10;
    }

    if (neg)
        buf[len++] = '-';

    /* Reverse the digit sequence in-place. */
    int lo = 0, hi = len - 1;
    while (lo < hi) {
        char tmp  = buf[lo];
        buf[lo++] = buf[hi];
        buf[hi--] = tmp;
    }

    /* Heap-allocate the result string (+1 for null terminator). */
    char *s = (char *)mem_alloc((usize)(len + 1));
    if (!s) return;

    for (int i = 0; i < len; i++)
        s[i] = buf[i];
    s[len] = '\0';

    *out = s;
}
