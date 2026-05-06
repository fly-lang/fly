/*===-- lib/fly/str.c - fly.str native implementations -----------------===
 *
 * Functions in this file implement fly.str primitives.
 * Compiled by the host C compiler, linked into libFlyLib.a.
 *
 * Calling convention — see String.h header.
 *
 * Name mangling:  _F<ns_len><ns_flat><fn_len><fn><param-type-codes>
 *   namespace fly.str → ns_flat = "fly_str" (7 chars)
 *   bool   → _b      byte  → _y    short → _s    int  → _i
 *   ulong  → _ul     long  → _l    float → _f    double → _d
 *   string → _Ss     error → _e    array → _A<elem>
 *===----------------------------------------------------------------------===*/

#include "str.h"

#include "platform/Runtime.h"

/* Allocator symbols — declare with unsigned long (= size_t on LP64) to match the
 * compiler's builtin signature and avoid -Wimplicit-function-declaration. */
extern void *malloc (unsigned long size);
extern void *realloc(void *ptr, unsigned long size);
extern void  free   (void *ptr);

/* ── Internal helpers ─────────────────────────────────────────────────────── */

static int fly_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static char fly_toupper(char c) {
    return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
}

static char fly_tolower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
}

/* Return index of first occurrence of needle[0..nlen) in hay[0..hlen), or -1. */
static int fly_find(const char *hay, int hlen, const char *needle, int nlen) {
    if (nlen == 0) return 0;
    if (nlen > hlen) return -1;
    int limit = hlen - nlen;
    for (int i = 0; i <= limit; i++) {
        int match = 1;
        for (int j = 0; j < nlen; j++) {
            if (hay[i + j] != needle[j]) { match = 0; break; }
        }
        if (match) return i;
    }
    return -1;
}

/* Allocate a heap buffer of at least 1 byte; returns NULL on failure. */
static char *fly_alloc(int size) {
    return (char *)malloc((usize)(size > 0 ? size : 1));
}

/* Copy src[0..len) into a freshly malloc'd buffer; returns NULL on failure. */
static char *fly_dup(const char *src, int len) {
    char *buf = fly_alloc(len);
    if (!buf) return (void *)0;
    for (int i = 0; i < len; i++) buf[i] = src[i];
    return buf;
}

/* ── Query functions (write scalar result into *out) ─────────────────────── */

void _F7fly_str3len_Ss_i(void *err_ctx, const fly_string *src, int *out) {
    (void)err_ctx;
    if (out) *out = src ? src->size : 0;
}

void _F7fly_str7isEmpty_Ss_b(void *err_ctx, const fly_string *src, int *out) {
    (void)err_ctx;
    if (out) *out = !src || src->size == 0;
}

void _F7fly_str8contains_Ss_Ss_b(void *err_ctx, const fly_string *src, const fly_string *sub, int *out) {
    (void)err_ctx;
    if (out) *out = (src && sub) ? fly_find(src->ptr, src->size, sub->ptr, sub->size) >= 0 : 0;
}

void _F7fly_str10startsWith_Ss_Ss_b(void *err_ctx, const fly_string *src, const fly_string *prefix, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!src || !prefix || prefix->size > src->size) { *out = 0; return; }
    int match = 1;
    for (int i = 0; i < prefix->size; i++)
        if (src->ptr[i] != prefix->ptr[i]) { match = 0; break; }
    *out = match;
}

void _F7fly_str8endsWith_Ss_Ss_b(void *err_ctx, const fly_string *src, const fly_string *suffix, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!src || !suffix || suffix->size > src->size) { *out = 0; return; }
    int offset = src->size - suffix->size;
    int match = 1;
    for (int i = 0; i < suffix->size; i++)
        if (src->ptr[offset + i] != suffix->ptr[i]) { match = 0; break; }
    *out = match;
}

void _F7fly_str7indexOf_Ss_Ss_i(void *err_ctx, const fly_string *src, const fly_string *sub, int *out) {
    (void)err_ctx;
    if (out) *out = (src && sub) ? fly_find(src->ptr, src->size, sub->ptr, sub->size) : -1;
}

void _F7fly_str11lastIndexOf_Ss_Ss_i(void *err_ctx, const fly_string *src, const fly_string *sub, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!src || !sub || sub->size == 0 || sub->size > src->size) { *out = -1; return; }
    int last = -1;
    int limit = src->size - sub->size;
    for (int i = 0; i <= limit; i++) {
        int match = 1;
        for (int j = 0; j < sub->size; j++)
            if (src->ptr[i + j] != sub->ptr[j]) { match = 0; break; }
        if (match) last = i;
    }
    *out = last;
}

void _F7fly_str6equals_Ss_Ss_b(void *err_ctx, const fly_string *a, const fly_string *b, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!a || !b) { *out = (!a && !b); return; }
    if (a->size != b->size) { *out = 0; return; }
    int eq = 1;
    for (int i = 0; i < a->size; i++)
        if (a->ptr[i] != b->ptr[i]) { eq = 0; break; }
    *out = eq;
}

void _F7fly_str16equalsIgnoreCase_Ss_Ss_b(void *err_ctx, const fly_string *a, const fly_string *b, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!a || !b) { *out = (!a && !b); return; }
    if (a->size != b->size) { *out = 0; return; }
    int eq = 1;
    for (int i = 0; i < a->size; i++)
        if (fly_tolower(a->ptr[i]) != fly_tolower(b->ptr[i])) { eq = 0; break; }
    *out = eq;
}

void _F7fly_str5count_Ss_Ss_i(void *err_ctx, const fly_string *src, const fly_string *sub, int *out) {
    (void)err_ctx;
    if (!out) return;
    if (!src || !sub || sub->size == 0 || sub->size > src->size) { *out = 0; return; }
    int n = 0, i = 0;
    while (i <= src->size - sub->size) {
        int pos = fly_find(src->ptr + i, src->size - i, sub->ptr, sub->size);
        if (pos < 0) break;
        n++;
        i += pos + sub->size;
    }
    *out = n;
}

/* ── Transform functions ──────────────────────────────────────────────────── */

void _F7fly_str7toUpper_Ss_Ss(void *err_ctx, const fly_string *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;
    char *buf = fly_alloc(src->size);
    if (!buf) return;
    for (int i = 0; i < src->size; i++) buf[i] = fly_toupper(src->ptr[i]);
    out->ptr  = buf;
    out->size = src->size;
}

void _F7fly_str7toLower_Ss_Ss(void *err_ctx, const fly_string *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;
    char *buf = fly_alloc(src->size);
    if (!buf) return;
    for (int i = 0; i < src->size; i++) buf[i] = fly_tolower(src->ptr[i]);
    out->ptr  = buf;
    out->size = src->size;
}

void _F7fly_str4trim_Ss_Ss(void *err_ctx, const fly_string *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;
    int lo = 0, hi = src->size;
    while (lo < hi && fly_isspace(src->ptr[lo])) lo++;
    while (hi > lo && fly_isspace(src->ptr[hi - 1])) hi--;
    int len = hi - lo;
    char *buf = fly_alloc(len);
    if (!buf) return;
    for (int i = 0; i < len; i++) buf[i] = src->ptr[lo + i];
    out->ptr  = buf;
    out->size = len;
}

void _F7fly_str8trimLeft_Ss_Ss(void *err_ctx, const fly_string *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;
    int lo = 0;
    while (lo < src->size && fly_isspace(src->ptr[lo])) lo++;
    int len = src->size - lo;
    char *buf = fly_alloc(len);
    if (!buf) return;
    for (int i = 0; i < len; i++) buf[i] = src->ptr[lo + i];
    out->ptr  = buf;
    out->size = len;
}

void _F7fly_str9trimRight_Ss_Ss(void *err_ctx, const fly_string *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;
    int hi = src->size;
    while (hi > 0 && fly_isspace(src->ptr[hi - 1])) hi--;
    char *buf = fly_alloc(hi);
    if (!buf) return;
    for (int i = 0; i < hi; i++) buf[i] = src->ptr[i];
    out->ptr  = buf;
    out->size = hi;
}

void _F7fly_str9substring_Ss_i_i_Ss(void *err_ctx, const fly_string *src,
                                     const int *start, const int *end, fly_string *out) {
    (void)err_ctx;
    if (!src || !start || !end || !out) return;
    int s = *start, e = *end;
    if (s < 0)         s = 0;
    if (e > src->size) e = src->size;
    if (s > e)         s = e;
    int len = e - s;
    char *buf = fly_alloc(len);
    if (!buf) return;
    for (int i = 0; i < len; i++) buf[i] = src->ptr[s + i];
    out->ptr  = buf;
    out->size = len;
}

void _F7fly_str7replace_Ss_Ss_Ss_Ss(void *err_ctx, const fly_string *src,
                                     const fly_string *from, const fly_string *to, fly_string *out) {
    (void)err_ctx;
    if (!src || !from || !to || !out) return;

    /* Degenerate: empty needle — return copy of src */
    if (from->size == 0) {
        char *buf = fly_dup(src->ptr, src->size);
        if (!buf) return;
        out->ptr = buf; out->size = src->size;
        return;
    }

    /* Count occurrences to compute result size */
    int occ = 0;
    _F7fly_str5count_Ss_Ss_i(err_ctx, src, from, &occ);
    int new_size = src->size + occ * (to->size - from->size);
    char *buf = fly_alloc(new_size);
    if (!buf) return;

    int ri = 0, wi = 0;
    while (ri < src->size) {
        int pos = fly_find(src->ptr + ri, src->size - ri, from->ptr, from->size);
        if (pos < 0) {
            int rem = src->size - ri;
            for (int k = 0; k < rem; k++) buf[wi++] = src->ptr[ri + k];
            break;
        }
        for (int k = 0; k < pos; k++) buf[wi++] = src->ptr[ri + k];
        ri += pos;
        for (int k = 0; k < to->size; k++) buf[wi++] = to->ptr[k];
        ri += from->size;
    }
    out->ptr  = buf;
    out->size = new_size;
}

void _F7fly_str6repeat_Ss_i_Ss(void *err_ctx, const fly_string *src, const int *count, fly_string *out) {
    (void)err_ctx;
    if (!src || !count || !out) return;
    int n = *count;
    if (n <= 0 || src->size == 0) {
        out->ptr  = fly_alloc(0);
        out->size = 0;
        return;
    }
    int new_size = src->size * n;
    char *buf = fly_alloc(new_size);
    if (!buf) return;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < src->size; j++)
            buf[i * src->size + j] = src->ptr[j];
    out->ptr  = buf;
    out->size = new_size;
}

void _F7fly_str6concat_Ss_Ss_Ss(void *err_ctx, const fly_string *a, const fly_string *b, fly_string *out) {
    (void)err_ctx;
    if (!a || !b || !out) return;
    int new_size = a->size + b->size;
    char *buf = fly_alloc(new_size);
    if (!buf) return;
    for (int i = 0; i < a->size; i++) buf[i]            = a->ptr[i];
    for (int i = 0; i < b->size; i++) buf[a->size + i]  = b->ptr[i];
    out->ptr  = buf;
    out->size = new_size;
}

/* ── Conversion ───────────────────────────────────────────────────────────── */

void _F7fly_str7convert_i_Ss(void *err_ctx, const int *src, fly_string *out) {
    (void)err_ctx;
    if (!src || !out) return;

    int n = *src;

    if (n == 0) {
        char *s = fly_alloc(1);
        if (!s) return;
        s[0]     = '0';
        out->ptr  = s;
        out->size = 1;
        return;
    }

    char buf[12];
    int  len = 0;
    int  neg = (n < 0);

    unsigned int un = neg ? (unsigned int)(0 - (unsigned int)n) : (unsigned int)n;

    while (un > 0) {
        buf[len++] = (char)('0' + (un % 10));
        un /= 10;
    }
    if (neg) buf[len++] = '-';

    for (int lo = 0, hi = len - 1; lo < hi; lo++, hi--) {
        char tmp = buf[lo]; buf[lo] = buf[hi]; buf[hi] = tmp;
    }

    char *s = fly_alloc(len);
    if (!s) return;
    for (int i = 0; i < len; i++) s[i] = buf[i];

    out->ptr  = s;
    out->size = len;
}
