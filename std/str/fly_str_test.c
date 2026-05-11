/*===-- std/str/fly_str_test.c - freestanding tests for fly.str --------===
 *
 * Freestanding test binary: no libc headers, no system headers.
 * I/O via Linux x86-64 write(2)/exit(2) syscalls inline.
 * malloc/free come from libc linked explicitly (-lc) because fly_str.c
 * allocates heap memory for transform functions.
 * Entry point: _start  (no CRT required).
 *
 * Build:
 *   clang -O2 -nostdinc -nostdlib -nostartfiles \
 *         fly_str_test.c fly_str.c -I. -I../../runtime -lc -o fly_str_test
 *   ./fly_str_test && echo "ALL PASSED"
 *===----------------------------------------------------------------------===*/

#include "fly_str.h"

#if __STDC_VERSION__ < 202311L
typedef _Bool bool;
#define true  ((bool)1)
#define false ((bool)0)
#endif

/* Transform functions allocate via malloc; the test frees returned buffers. */
extern void free(void *ptr);

/* ── Minimal Linux x86-64 syscall I/O ─────────────────────────────────────── */

static __attribute__((noinline)) void fly_write(const char *buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"(1L), "S"(buf), "d"(len)
        : "rcx", "r11", "memory");
}

static void fly_puts(const char *s) {
    long len = 0;
    while (s[len]) len++;
    fly_write(s, len);
}

static __attribute__((noreturn, noinline)) void fly_exit(int code) {
    __asm__ volatile("syscall"
        : : "a"(60L), "D"((long)code)
        : "rcx", "r11", "memory");
    __builtin_unreachable();
}

/* ── Test framework ────────────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const char *name) {
    if (cond) {
        g_pass++;
    } else {
        g_fail++;
        fly_puts("FAIL: ");
        fly_puts(name);
        fly_write("\n", 1);
    }
}

/* ── String helpers ────────────────────────────────────────────────────────── */

/* Build a fly_string that borrows a string literal (no allocation). */
static fly_string mkstr(const char *s) {
    fly_string fs;
    int len = 0;
    while (s[len]) len++;
    fs.ptr  = (char *)s;
    fs.size = len;
    return fs;
}

/* Compare a fly_string against a C string literal byte-for-byte. */
static bool str_eq(const fly_string *fs, const char *expected) {
    int len = 0;
    while (expected[len]) len++;
    if (fs->size != len) return false;
    for (int i = 0; i < len; i++)
        if (fs->ptr[i] != expected[i]) return false;
    return true;
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_len(void) {
    fly_string s = mkstr("hello");
    fly_string e = mkstr("");
    int out = 0;

    _F7fly_str3len_Ss_i((void *)0, &s, &out); check(out == 5, "len hello=5");
    _F7fly_str3len_Ss_i((void *)0, &e, &out); check(out == 0, "len empty=0");
}

static void test_is_empty(void) {
    fly_string empty = mkstr("");
    fly_string s     = mkstr("x");
    int out = 0;

    _F7fly_str7isEmpty_Ss_b((void *)0, &empty, &out); check(out == 1, "isEmpty(\"\")");
    _F7fly_str7isEmpty_Ss_b((void *)0, &s,     &out); check(out == 0, "isEmpty(\"x\")");
}

static void test_contains(void) {
    fly_string hay     = mkstr("hello world");
    fly_string found   = mkstr("world");
    fly_string missing = mkstr("xyz");
    int out = 0;

    _F7fly_str8contains_Ss_Ss_b((void *)0, &hay, &found,   &out); check(out == 1, "contains found");
    _F7fly_str8contains_Ss_Ss_b((void *)0, &hay, &missing, &out); check(out == 0, "contains missing");
}

static void test_starts_with(void) {
    fly_string s      = mkstr("hello");
    fly_string yes    = mkstr("hell");
    fly_string no     = mkstr("world");
    fly_string longer = mkstr("hello world");
    int out = 0;

    _F7fly_str10startsWith_Ss_Ss_b((void *)0, &s, &yes,    &out); check(out == 1, "startsWith yes");
    _F7fly_str10startsWith_Ss_Ss_b((void *)0, &s, &no,     &out); check(out == 0, "startsWith no");
    _F7fly_str10startsWith_Ss_Ss_b((void *)0, &s, &longer, &out); check(out == 0, "startsWith longer");
}

static void test_ends_with(void) {
    fly_string s   = mkstr("hello");
    fly_string yes = mkstr("llo");
    fly_string no  = mkstr("hel");
    int out = 0;

    _F7fly_str8endsWith_Ss_Ss_b((void *)0, &s, &yes, &out); check(out == 1, "endsWith yes");
    _F7fly_str8endsWith_Ss_Ss_b((void *)0, &s, &no,  &out); check(out == 0, "endsWith no");
}

static void test_index_of(void) {
    fly_string s    = mkstr("abcabc");
    fly_string sub  = mkstr("bc");
    fly_string miss = mkstr("xyz");
    int out = 0;

    _F7fly_str7indexOf_Ss_Ss_i((void *)0, &s, &sub,  &out); check(out ==  1, "indexOf found=1");
    _F7fly_str7indexOf_Ss_Ss_i((void *)0, &s, &miss, &out); check(out == -1, "indexOf miss=-1");
}

static void test_last_index_of(void) {
    fly_string s    = mkstr("abcabc");
    fly_string sub  = mkstr("bc");
    fly_string miss = mkstr("xyz");
    int out = 0;

    _F7fly_str11lastIndexOf_Ss_Ss_i((void *)0, &s, &sub,  &out); check(out ==  4, "lastIndexOf=4");
    _F7fly_str11lastIndexOf_Ss_Ss_i((void *)0, &s, &miss, &out); check(out == -1, "lastIndexOf miss=-1");
}

static void test_equals(void) {
    fly_string a = mkstr("abc");
    fly_string b = mkstr("abc");
    fly_string c = mkstr("def");
    int out = 0;

    _F7fly_str6equals_Ss_Ss_b((void *)0, &a, &b, &out); check(out == 1, "equals abc==abc");
    _F7fly_str6equals_Ss_Ss_b((void *)0, &a, &c, &out); check(out == 0, "equals abc!=def");
}

static void test_equals_ignore_case(void) {
    fly_string a = mkstr("Hello");
    fly_string b = mkstr("hello");
    fly_string c = mkstr("world");
    int out = 0;

    _F7fly_str16equalsIgnoreCase_Ss_Ss_b((void *)0, &a, &b, &out); check(out == 1, "eqIC Hello==hello");
    _F7fly_str16equalsIgnoreCase_Ss_Ss_b((void *)0, &a, &c, &out); check(out == 0, "eqIC Hello!=world");
}

static void test_count(void) {
    fly_string s    = mkstr("abcabcabc");
    fly_string sub  = mkstr("abc");
    fly_string miss = mkstr("xyz");
    int out = 0;

    _F7fly_str5count_Ss_Ss_i((void *)0, &s, &sub,  &out); check(out == 3, "count abc=3");
    _F7fly_str5count_Ss_Ss_i((void *)0, &s, &miss, &out); check(out == 0, "count xyz=0");
}

static void test_to_upper(void) {
    fly_string s   = mkstr("hello");
    fly_string out = {(void *)0, 0};
    _F7fly_str7toUpper_Ss_Ss((void *)0, &s, &out);
    check(out.ptr != (void *)0,       "toUpper non-null");
    check(str_eq(&out, "HELLO"),      "toUpper hello→HELLO");
    free(out.ptr);
}

static void test_to_lower(void) {
    fly_string s   = mkstr("HELLO");
    fly_string out = {(void *)0, 0};
    _F7fly_str7toLower_Ss_Ss((void *)0, &s, &out);
    check(out.ptr != (void *)0,       "toLower non-null");
    check(str_eq(&out, "hello"),      "toLower HELLO→hello");
    free(out.ptr);
}

static void test_trim(void) {
    fly_string s   = mkstr("  hello  ");
    fly_string out = {(void *)0, 0};
    _F7fly_str4trim_Ss_Ss((void *)0, &s, &out);
    check(out.ptr != (void *)0,  "trim non-null");
    check(str_eq(&out, "hello"), "trim spaces→hello");
    free(out.ptr);

    fly_string spaces = mkstr("   ");
    fly_string out2   = {(void *)0, 0};
    _F7fly_str4trim_Ss_Ss((void *)0, &spaces, &out2);
    check(out2.ptr != (void *)0, "trim spaces-only non-null");
    check(out2.size == 0,        "trim spaces-only size=0");
    free(out2.ptr);
}

static void test_trim_left(void) {
    fly_string s   = mkstr("  hello  ");
    fly_string out = {(void *)0, 0};
    _F7fly_str8trimLeft_Ss_Ss((void *)0, &s, &out);
    check(out.ptr != (void *)0,       "trimLeft non-null");
    check(str_eq(&out, "hello  "),    "trimLeft result");
    free(out.ptr);
}

static void test_trim_right(void) {
    fly_string s   = mkstr("  hello  ");
    fly_string out = {(void *)0, 0};
    _F7fly_str9trimRight_Ss_Ss((void *)0, &s, &out);
    check(out.ptr != (void *)0,       "trimRight non-null");
    check(str_eq(&out, "  hello"),    "trimRight result");
    free(out.ptr);
}

static void test_substring(void) {
    fly_string s    = mkstr("hello");
    fly_string out  = {(void *)0, 0};
    int start = 1, end = 4;
    _F7fly_str9substring_Ss_i_i_Ss((void *)0, &s, &start, &end, &out);
    check(out.ptr != (void *)0,  "substring non-null");
    check(str_eq(&out, "ell"),   "substring [1,4]→ell");
    free(out.ptr);

    fly_string out2 = {(void *)0, 0};
    int s2 = 0, e2 = 5;
    _F7fly_str9substring_Ss_i_i_Ss((void *)0, &s, &s2, &e2, &out2);
    check(out2.ptr != (void *)0,  "substring full non-null");
    check(str_eq(&out2, "hello"), "substring [0,5]→hello");
    free(out2.ptr);
}

static void test_replace(void) {
    fly_string s    = mkstr("hello world");
    fly_string from = mkstr("world");
    fly_string to   = mkstr("fly");
    fly_string out  = {(void *)0, 0};
    _F7fly_str7replace_Ss_Ss_Ss_Ss((void *)0, &s, &from, &to, &out);
    check(out.ptr != (void *)0,        "replace non-null");
    check(str_eq(&out, "hello fly"),   "replace world→fly");
    free(out.ptr);

    fly_string s2   = mkstr("aabbaa");
    fly_string frm2 = mkstr("aa");
    fly_string to2  = mkstr("X");
    fly_string out2 = {(void *)0, 0};
    _F7fly_str7replace_Ss_Ss_Ss_Ss((void *)0, &s2, &frm2, &to2, &out2);
    check(out2.ptr != (void *)0,  "replace multi non-null");
    check(str_eq(&out2, "XbbX"),  "replace aa→X");
    free(out2.ptr);
}

static void test_repeat(void) {
    fly_string s   = mkstr("ab");
    fly_string out = {(void *)0, 0};
    int n = 3;
    _F7fly_str6repeat_Ss_i_Ss((void *)0, &s, &n, &out);
    check(out.ptr != (void *)0,    "repeat non-null");
    check(str_eq(&out, "ababab"),  "repeat ab×3");
    free(out.ptr);

    fly_string out2 = {(void *)0, 0};
    int zero = 0;
    _F7fly_str6repeat_Ss_i_Ss((void *)0, &s, &zero, &out2);
    check(out2.ptr != (void *)0,  "repeat×0 non-null");
    check(out2.size == 0,         "repeat×0 size=0");
    free(out2.ptr);
}

static void test_concat(void) {
    fly_string a   = mkstr("hello");
    fly_string b   = mkstr(" world");
    fly_string out = {(void *)0, 0};
    _F7fly_str6concat_Ss_Ss_Ss((void *)0, &a, &b, &out);
    check(out.ptr != (void *)0,        "concat non-null");
    check(str_eq(&out, "hello world"), "concat hello+world");
    free(out.ptr);
}

static void test_convert(void) {
    fly_string out = {(void *)0, 0};
    int n = 42;
    _F7fly_str7convert_i_Ss((void *)0, &n, &out);
    check(out.ptr != (void *)0,  "convert 42 non-null");
    check(str_eq(&out, "42"),    "convert 42→\"42\"");
    free(out.ptr);

    fly_string out2 = {(void *)0, 0};
    int neg = -7;
    _F7fly_str7convert_i_Ss((void *)0, &neg, &out2);
    check(out2.ptr != (void *)0, "convert -7 non-null");
    check(str_eq(&out2, "-7"),   "convert -7→\"-7\"");
    free(out2.ptr);

    fly_string out3 = {(void *)0, 0};
    int zero = 0;
    _F7fly_str7convert_i_Ss((void *)0, &zero, &out3);
    check(out3.ptr != (void *)0, "convert 0 non-null");
    check(str_eq(&out3, "0"),    "convert 0→\"0\"");
    free(out3.ptr);
}

/* ── Summary output ────────────────────────────────────────────────────────── */

static void fly_put_u64(unsigned long long v) {
    if (v == 0) { fly_write("0", 1); return; }
    char buf[20];
    int len = 0;
    while (v) { buf[len++] = (char)('0' + (int)(v % 10)); v /= 10; }
    for (int lo = 0, hi = len - 1; lo < hi; lo++, hi--) {
        char t = buf[lo]; buf[lo] = buf[hi]; buf[hi] = t;
    }
    fly_write(buf, (long)len);
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

static void run_tests(void);

__attribute__((naked)) void _start(void) {
    __asm__ volatile(
        "andq  $-16, %%rsp\n"
        "callq %P0\n"
        : : "i"(run_tests)
    );
}

static void run_tests(void) {
    test_len();
    test_is_empty();
    test_contains();
    test_starts_with();
    test_ends_with();
    test_index_of();
    test_last_index_of();
    test_equals();
    test_equals_ignore_case();
    test_count();
    test_to_upper();
    test_to_lower();
    test_trim();
    test_trim_left();
    test_trim_right();
    test_substring();
    test_replace();
    test_repeat();
    test_concat();
    test_convert();

    unsigned long long total = (unsigned long long)(g_pass + g_fail);
    fly_puts("fly_str: ");
    fly_put_u64((unsigned long long)g_pass);
    fly_write("/", 1);
    fly_put_u64(total);
    fly_puts(" passed");
    if (g_fail) {
        fly_puts("  (");
        fly_put_u64((unsigned long long)g_fail);
        fly_puts(" FAILED)");
    }
    fly_write("\n", 1);
    fly_exit(g_fail ? 1 : 0);
}
