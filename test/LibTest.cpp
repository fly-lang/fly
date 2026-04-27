//===-- test/LibTest.cpp - Unit tests for fly.str C implementations -------===//
//
// Tests call the mangled C functions in libFlyLib directly, bypassing the
// Fly compiler.  Each test constructs fly_string values on the stack (ptr
// pointing into a string literal, size set manually) and asserts on the
// output parameter value.
//
// Ownership rule: transform functions (toUpper, trim, …) heap-allocate
// out.ptr via malloc.  The test frees it with std::free after checking.
//===----------------------------------------------------------------------===//

#include <gtest/gtest.h>
#include <cstdlib>   // std::free

extern "C" {
#include "../lib/String.h"
}

/* Build a fly_string that borrows a C string literal (read-only). */
static fly_string str(const char *s) {
    fly_string fs;
    int len = 0;
    while (s[len]) len++;
    fs.ptr  = const_cast<char *>(s);
    fs.size = len;
    return fs;
}

/* Compare a fly_string against a C string literal byte-for-byte. */
static bool str_eq(const fly_string &fs, const char *expected) {
    int len = 0;
    while (expected[len]) len++;
    if (fs.size != len) return false;
    for (int i = 0; i < len; i++)
        if (fs.ptr[i] != expected[i]) return false;
    return true;
}

// ── 1. len ────────────────────────────────────────────────────────────────

TEST(FlyStr, Len) {
    fly_string s = str("hello");
    int out = 0;
    _F7fly_str3len_Ss_i(nullptr, &s, &out);
    EXPECT_EQ(out, 5);

    fly_string empty = str("");
    _F7fly_str3len_Ss_i(nullptr, &empty, &out);
    EXPECT_EQ(out, 0);
}

// ── 2. isEmpty ───────────────────────────────────────────────────────────────

TEST(FlyStr, IsEmpty) {
    fly_string empty = str("");
    int out = 0;
    _F7fly_str7isEmpty_Ss_b(nullptr, &empty, &out);
    EXPECT_EQ(out, 1);

    fly_string s = str("x");
    _F7fly_str7isEmpty_Ss_b(nullptr, &s, &out);
    EXPECT_EQ(out, 0);
}

// ── 3. contains ──────────────────────────────────────────────────────────────

TEST(FlyStr, Contains) {
    fly_string haystack = str("hello world");
    fly_string found    = str("world");
    fly_string missing  = str("xyz");
    int out = 0;

    _F7fly_str8contains_Ss_Ss_b(nullptr, &haystack, &found, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str8contains_Ss_Ss_b(nullptr, &haystack, &missing, &out);
    EXPECT_EQ(out, 0);
}

// ── 4. startsWith ────────────────────────────────────────────────────────────

TEST(FlyStr, StartsWith) {
    fly_string s      = str("hello");
    fly_string yes    = str("hell");
    fly_string no     = str("world");
    fly_string longer = str("hello world");
    int out = 0;

    _F7fly_str10startsWith_Ss_Ss_b(nullptr, &s, &yes, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str10startsWith_Ss_Ss_b(nullptr, &s, &no, &out);
    EXPECT_EQ(out, 0);

    _F7fly_str10startsWith_Ss_Ss_b(nullptr, &s, &longer, &out);
    EXPECT_EQ(out, 0);
}

// ── 5. endsWith ──────────────────────────────────────────────────────────────

TEST(FlyStr, EndsWith) {
    fly_string s   = str("hello");
    fly_string yes = str("llo");
    fly_string no  = str("hel");
    int out = 0;

    _F7fly_str8endsWith_Ss_Ss_b(nullptr, &s, &yes, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str8endsWith_Ss_Ss_b(nullptr, &s, &no, &out);
    EXPECT_EQ(out, 0);
}

// ── 6. indexOf ───────────────────────────────────────────────────────────────

TEST(FlyStr, IndexOf) {
    fly_string s    = str("abcabc");
    fly_string sub  = str("bc");
    fly_string miss = str("xyz");
    int out = 0;

    _F7fly_str7indexOf_Ss_Ss_i(nullptr, &s, &sub, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str7indexOf_Ss_Ss_i(nullptr, &s, &miss, &out);
    EXPECT_EQ(out, -1);
}

// ── 7. lastIndexOf ───────────────────────────────────────────────────────────

TEST(FlyStr, LastIndexOf) {
    fly_string s    = str("abcabc");
    fly_string sub  = str("bc");
    fly_string miss = str("xyz");
    int out = 0;

    _F7fly_str11lastIndexOf_Ss_Ss_i(nullptr, &s, &sub, &out);
    EXPECT_EQ(out, 4);

    _F7fly_str11lastIndexOf_Ss_Ss_i(nullptr, &s, &miss, &out);
    EXPECT_EQ(out, -1);
}

// ── 8. equals ────────────────────────────────────────────────────────────────

TEST(FlyStr, Equals) {
    fly_string a = str("abc");
    fly_string b = str("abc");
    fly_string c = str("def");
    int out = 0;

    _F7fly_str6equals_Ss_Ss_b(nullptr, &a, &b, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str6equals_Ss_Ss_b(nullptr, &a, &c, &out);
    EXPECT_EQ(out, 0);
}

// ── 9. equalsIgnoreCase ───────────────────────────────────────────────────────

TEST(FlyStr, EqualsIgnoreCase) {
    fly_string a = str("Hello");
    fly_string b = str("hello");
    fly_string c = str("world");
    int out = 0;

    _F7fly_str16equalsIgnoreCase_Ss_Ss_b(nullptr, &a, &b, &out);
    EXPECT_EQ(out, 1);

    _F7fly_str16equalsIgnoreCase_Ss_Ss_b(nullptr, &a, &c, &out);
    EXPECT_EQ(out, 0);
}

// ── 10. count ────────────────────────────────────────────────────────────────

TEST(FlyStr, Count) {
    fly_string s    = str("abcabcabc");
    fly_string sub  = str("abc");
    fly_string miss = str("xyz");
    int out = 0;

    _F7fly_str5count_Ss_Ss_i(nullptr, &s, &sub, &out);
    EXPECT_EQ(out, 3);

    _F7fly_str5count_Ss_Ss_i(nullptr, &s, &miss, &out);
    EXPECT_EQ(out, 0);
}

// ── 11. toUpper ──────────────────────────────────────────────────────────────

TEST(FlyStr, ToUpper) {
    fly_string s   = str("hello");
    fly_string out = {nullptr, 0};
    _F7fly_str7toUpper_Ss_Ss(nullptr, &s, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "HELLO"));
    std::free(out.ptr);
}

// ── 12. toLower ──────────────────────────────────────────────────────────────

TEST(FlyStr, ToLower) {
    fly_string s   = str("HELLO");
    fly_string out = {nullptr, 0};
    _F7fly_str7toLower_Ss_Ss(nullptr, &s, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "hello"));
    std::free(out.ptr);
}

// ── 13. trim ─────────────────────────────────────────────────────────────────

TEST(FlyStr, Trim) {
    fly_string s   = str("  hello  ");
    fly_string out = {nullptr, 0};
    _F7fly_str4trim_Ss_Ss(nullptr, &s, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "hello"));
    std::free(out.ptr);

    fly_string spaces = str("   ");
    fly_string out2   = {nullptr, 0};
    _F7fly_str4trim_Ss_Ss(nullptr, &spaces, &out2);
    ASSERT_NE(out2.ptr, nullptr);
    EXPECT_EQ(out2.size, 0);
    std::free(out2.ptr);
}

// ── 14. trimLeft ─────────────────────────────────────────────────────────────

TEST(FlyStr, TrimLeft) {
    fly_string s   = str("  hello  ");
    fly_string out = {nullptr, 0};
    _F7fly_str8trimLeft_Ss_Ss(nullptr, &s, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "hello  "));
    std::free(out.ptr);
}

// ── 15. trimRight ────────────────────────────────────────────────────────────

TEST(FlyStr, TrimRight) {
    fly_string s   = str("  hello  ");
    fly_string out = {nullptr, 0};
    _F7fly_str9trimRight_Ss_Ss(nullptr, &s, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "  hello"));
    std::free(out.ptr);
}

// ── 16. substring ────────────────────────────────────────────────────────────

TEST(FlyStr, Substring) {
    fly_string s   = str("hello");
    fly_string out = {nullptr, 0};
    int start = 1, end = 4;
    _F7fly_str9substring_Ss_i_i_Ss(nullptr, &s, &start, &end, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "ell"));
    std::free(out.ptr);

    fly_string out2 = {nullptr, 0};
    int s2 = 0, e2 = 5;
    _F7fly_str9substring_Ss_i_i_Ss(nullptr, &s, &s2, &e2, &out2);
    ASSERT_NE(out2.ptr, nullptr);
    EXPECT_TRUE(str_eq(out2, "hello"));
    std::free(out2.ptr);
}

// ── 17. replace ──────────────────────────────────────────────────────────────

TEST(FlyStr, Replace) {
    fly_string s    = str("hello world");
    fly_string from = str("world");
    fly_string to   = str("fly");
    fly_string out  = {nullptr, 0};
    _F7fly_str7replace_Ss_Ss_Ss_Ss(nullptr, &s, &from, &to, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "hello fly"));
    std::free(out.ptr);

    fly_string s2   = str("aabbaa");
    fly_string frm2 = str("aa");
    fly_string to2  = str("X");
    fly_string out2 = {nullptr, 0};
    _F7fly_str7replace_Ss_Ss_Ss_Ss(nullptr, &s2, &frm2, &to2, &out2);
    ASSERT_NE(out2.ptr, nullptr);
    EXPECT_TRUE(str_eq(out2, "XbbX"));
    std::free(out2.ptr);
}

// ── 18. repeat ───────────────────────────────────────────────────────────────

TEST(FlyStr, Repeat) {
    fly_string s   = str("ab");
    fly_string out = {nullptr, 0};
    int n = 3;
    _F7fly_str6repeat_Ss_i_Ss(nullptr, &s, &n, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "ababab"));
    std::free(out.ptr);

    fly_string out2 = {nullptr, 0};
    int zero = 0;
    _F7fly_str6repeat_Ss_i_Ss(nullptr, &s, &zero, &out2);
    ASSERT_NE(out2.ptr, nullptr);
    EXPECT_EQ(out2.size, 0);
    std::free(out2.ptr);
}

// ── 19. concat ───────────────────────────────────────────────────────────────

TEST(FlyStr, Concat) {
    fly_string a   = str("hello");
    fly_string b   = str(" world");
    fly_string out = {nullptr, 0};
    _F7fly_str6concat_Ss_Ss_Ss(nullptr, &a, &b, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "hello world"));
    std::free(out.ptr);
}

// ── 20. convert ──────────────────────────────────────────────────────────────

TEST(FlyStr, Convert) {
    fly_string out = {nullptr, 0};
    int n = 42;
    _F7fly_str7convert_i_Ss(nullptr, &n, &out);

    ASSERT_NE(out.ptr, nullptr);
    EXPECT_TRUE(str_eq(out, "42"));
    std::free(out.ptr);

    fly_string out2 = {nullptr, 0};
    int neg = -7;
    _F7fly_str7convert_i_Ss(nullptr, &neg, &out2);
    ASSERT_NE(out2.ptr, nullptr);
    EXPECT_TRUE(str_eq(out2, "-7"));
    std::free(out2.ptr);

    fly_string out3 = {nullptr, 0};
    int zero = 0;
    _F7fly_str7convert_i_Ss(nullptr, &zero, &out3);
    ASSERT_NE(out3.ptr, nullptr);
    EXPECT_TRUE(str_eq(out3, "0"));
    std::free(out3.ptr);
}
