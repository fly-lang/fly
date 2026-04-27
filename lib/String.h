/*===-- lib/Fly/String.h - fly.str C types and declarations ---------------===
 *
 * Internal header shared between String.c and LibTest.cpp.
 * No libc headers — only fundamental C types.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_LIB_STRING_H
#define FLY_LIB_STRING_H

/* ── Fly string struct (mirrors LLVM IR: %string = { i8*, i32 }) ─────────── */

typedef struct {
    char *ptr;   /* heap-allocated byte buffer (not null-terminated) */
    int   size;  /* length in bytes                                   */
} fly_string;

/* ── Array-of-strings result (used by split) ─────────────────────────────── */

typedef struct {
    fly_string *items;
    int         count;
} fly_string_array;

/* ── Calling convention ──────────────────────────────────────────────────── *
 *  All Fly functions receive:
 *    arg 0 : void *err_ctx  — implicit error-handler context (unused here)
 *    arg N : T *param       — each explicit param passed by reference
 *  All functions return void; scalar results are written to an output param.
 *
 *  Name mangling: _F<ns_len><ns_flat><fn_len><fn><param-type-codes>
 *    namespace fly.str → ns_flat = "fly_str" (7 chars)
 *    bool → _b   byte → _y   short → _s    int  → _i
 *    ulong → _ul  long → _l   float → _f    double → _d
 *    string → _Ss  error → _e   array → _A<elem>
 *===----------------------------------------------------------------------=== */

/* ── Query functions (write scalar result into *out) ─────────────────────── */

void _F7fly_str3len_Ss_i              (void *err_ctx, const fly_string *src, int *out);
void _F7fly_str7isEmpty_Ss_b          (void *err_ctx, const fly_string *src, int *out);
void _F7fly_str8contains_Ss_Ss_b      (void *err_ctx, const fly_string *src, const fly_string *sub, int *out);
void _F7fly_str10startsWith_Ss_Ss_b   (void *err_ctx, const fly_string *src, const fly_string *prefix, int *out);
void _F7fly_str8endsWith_Ss_Ss_b      (void *err_ctx, const fly_string *src, const fly_string *suffix, int *out);
void _F7fly_str7indexOf_Ss_Ss_i       (void *err_ctx, const fly_string *src, const fly_string *sub, int *out);
void _F7fly_str11lastIndexOf_Ss_Ss_i  (void *err_ctx, const fly_string *src, const fly_string *sub, int *out);
void _F7fly_str6equals_Ss_Ss_b        (void *err_ctx, const fly_string *a,   const fly_string *b, int *out);
void _F7fly_str16equalsIgnoreCase_Ss_Ss_b(void *err_ctx, const fly_string *a, const fly_string *b, int *out);
void _F7fly_str5count_Ss_Ss_i         (void *err_ctx, const fly_string *src, const fly_string *sub, int *out);

/* ── Transform functions (heap-allocate result into *out, caller frees) ───── */

void _F7fly_str7toUpper_Ss_Ss      (void *err_ctx, const fly_string *src, fly_string *out);
void _F7fly_str7toLower_Ss_Ss      (void *err_ctx, const fly_string *src, fly_string *out);
void _F7fly_str4trim_Ss_Ss         (void *err_ctx, const fly_string *src, fly_string *out);
void _F7fly_str8trimLeft_Ss_Ss     (void *err_ctx, const fly_string *src, fly_string *out);
void _F7fly_str9trimRight_Ss_Ss    (void *err_ctx, const fly_string *src, fly_string *out);
void _F7fly_str9substring_Ss_i_i_Ss(void *err_ctx, const fly_string *src,
                                    const int *start, const int *end, fly_string *out);
void _F7fly_str7replace_Ss_Ss_Ss_Ss(void *err_ctx, const fly_string *src,
                                    const fly_string *from, const fly_string *to, fly_string *out);
void _F7fly_str6repeat_Ss_i_Ss     (void *err_ctx, const fly_string *src, const int *count, fly_string *out);
void _F7fly_str6concat_Ss_Ss_Ss    (void *err_ctx, const fly_string *a, const fly_string *b, fly_string *out);

/* ── Conversion ───────────────────────────────────────────────────────────── */

void _F7fly_str7convert_i_Ss       (void *err_ctx, const int *src, fly_string *out);

#endif /* FLY_LIB_STRING_H */
