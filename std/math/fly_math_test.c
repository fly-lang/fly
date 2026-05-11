/*===-- std/math/fly_math_test.c - freestanding tests for fly.math ------===
 *
 * Freestanding test binary: no libc, no system headers.
 * I/O via Linux x86-64 write(2) syscall inline.
 * Entry point: _start  (no CRT required).
 *
 * Build:
 *   clang -O2 -nostdinc -nostdlib fly_math_test.c fly_math.c \
 *         -I. -lm -o fly_math_test
 *   ./fly_math_test && echo "ALL PASSED"
 *===----------------------------------------------------------------------===*/

#include "fly_math.h"

/* ── Minimal Linux x86-64 syscall I/O ─────────────────────────────────────── */

static __attribute__((noinline)) void fly_write(const char* buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"(1L), "S"(buf), "d"(len)
        : "rcx", "r11", "memory");
}

static void fly_puts(const char* s) {
    long len = 0;
    while (s[len]) len++;
    fly_write(s, len);
}

static void fly_put_u64(uint64_t v) {
    if (v == 0) { fly_write("0", 1); return; }
    char buf[20];
    int len = 0;
    while (v) { buf[len++] = (char)('0' + (int)(v % 10)); v /= 10; }
    for (int lo = 0, hi = len - 1; lo < hi; lo++, hi--) {
        char t = buf[lo]; buf[lo] = buf[hi]; buf[hi] = t;
    }
    fly_write(buf, (long)len);
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

static void check(int cond, const char* name) {
    if (cond) {
        g_pass++;
    } else {
        g_fail++;
        fly_puts("FAIL: ");
        fly_puts(name);
        fly_write("\n", 1);
    }
}

static int approx(double a, double b) {
    double diff = a - b;
    if (diff < 0.0) diff = -diff;
    return diff <= 1e-9;
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_abs(void) {
    int64_t r; double d;
    int64_t pos = 5, neg = -7, zer = 0;
    double fpos = 3.14, fneg = -2.71, fzer = 0.0;

    _F8fly_math4absI_l_l((void*)0, &pos,  &r); check(r == 5,             "absI pos");
    _F8fly_math4absI_l_l((void*)0, &neg,  &r); check(r == 7,             "absI neg");
    _F8fly_math4absI_l_l((void*)0, &zer,  &r); check(r == 0,             "absI zero");
    _F8fly_math4absF_d_d((void*)0, &fpos, &d); check(approx(d, 3.14),    "absF pos");
    _F8fly_math4absF_d_d((void*)0, &fneg, &d); check(approx(d, 2.71),    "absF neg");
    _F8fly_math4absF_d_d((void*)0, &fzer, &d); check(approx(d, 0.0),     "absF zero");
}

static void test_sign(void) {
    int out;
    double pos = 5.0, neg = -3.0, zer = 0.0;
    _F8fly_math4sign_d_i((void*)0, &pos, &out); check(out ==  1, "sign pos");
    _F8fly_math4sign_d_i((void*)0, &neg, &out); check(out == -1, "sign neg");
    _F8fly_math4sign_d_i((void*)0, &zer, &out); check(out ==  0, "sign zero");
}

static void test_clamp(void) {
    int64_t ri; double rd;
    int64_t lo = 0, hi = 10, inside = 5, below = -3, above = 15;
    double flo = 0.0, fhi = 1.0, fin = 0.5, fbelow = -0.5, fabove = 1.5;

    _F8fly_math6clampI_l_l_l_l((void*)0, &inside, &lo, &hi, &ri); check(ri == 5,          "clampI inside");
    _F8fly_math6clampI_l_l_l_l((void*)0, &below,  &lo, &hi, &ri); check(ri == 0,          "clampI below");
    _F8fly_math6clampI_l_l_l_l((void*)0, &above,  &lo, &hi, &ri); check(ri == 10,         "clampI above");
    _F8fly_math6clampF_d_d_d_d((void*)0, &fin,    &flo, &fhi, &rd); check(approx(rd, 0.5),"clampF inside");
    _F8fly_math6clampF_d_d_d_d((void*)0, &fbelow, &flo, &fhi, &rd); check(approx(rd, 0.0),"clampF below");
    _F8fly_math6clampF_d_d_d_d((void*)0, &fabove, &flo, &fhi, &rd); check(approx(rd, 1.0),"clampF above");
}

static void test_gcd_lcm(void) {
    int64_t r;
    int64_t a12 = 12, b8 = 8, a15 = 15, b25 = 25;
    int64_t z = 0, one = 1;

    _F8fly_math3gcd_l_l_l((void*)0, &a12, &b8,  &r); check(r == 4,  "gcd(12,8)");
    _F8fly_math3gcd_l_l_l((void*)0, &a15, &b25, &r); check(r == 5,  "gcd(15,25)");
    _F8fly_math3gcd_l_l_l((void*)0, &z,   &b8,  &r); check(r == 8,  "gcd(0,8)");
    _F8fly_math3gcd_l_l_l((void*)0, &one, &b8,  &r); check(r == 1,  "gcd(1,8)");

    _F8fly_math3lcm_l_l_l((void*)0, &a12, &b8,  &r); check(r == 24, "lcm(12,8)");
    _F8fly_math3lcm_l_l_l((void*)0, &a15, &b25, &r); check(r == 75, "lcm(15,25)");
    _F8fly_math3lcm_l_l_l((void*)0, &z,   &b8,  &r); check(r == 0,  "lcm(0,8)");
}

static void test_sin_cos(void) {
    double s, c, one = 1.0, pihalf = FLY_PI / 2.0, zero = 0.0;
    double eps = 1e-10;
    int ok;

    _F8fly_math3sin_d_d((void*)0, &zero,   &s); check(approx(s, 0.0), "sin(0)");
    _F8fly_math3cos_d_d((void*)0, &zero,   &c); check(approx(c, 1.0), "cos(0)");
    _F8fly_math3sin_d_d((void*)0, &pihalf, &s); check(approx(s, 1.0), "sin(π/2)");
    _F8fly_math3cos_d_d((void*)0, &pihalf, &c); check(approx(c, 0.0), "cos(π/2)");

    double angles[4] = {0.3, 1.1, 2.5, 4.7};
    for (int i = 0; i < 4; i++) {
        _F8fly_math3sin_d_d((void*)0, &angles[i], &s);
        _F8fly_math3cos_d_d((void*)0, &angles[i], &c);
        double sum = s * s + c * c;
        _F8fly_math11approxEqual_d_d_d_b((void*)0, &sum, &one, &eps, &ok);
        check(ok != 0, "sin²+cos²=1");
    }
}

static void test_atan2(void) {
    double r;
    double y1 = 1.0, x1 = 0.0;
    double y2 = 0.0, x2 = -1.0;
    double y3 = -1.0, x3 = 0.0;
    double y4 = 1.0, x4 = 1.0;

    _F8fly_math5atan2_d_d_d((void*)0, &y1, &x1, &r); check(approx(r,  FLY_PI / 2.0), "atan2(1,0)=π/2");
    _F8fly_math5atan2_d_d_d((void*)0, &y2, &x2, &r); check(approx(r,  FLY_PI),       "atan2(0,-1)=π");
    _F8fly_math5atan2_d_d_d((void*)0, &y3, &x3, &r); check(approx(r, -FLY_PI / 2.0),"atan2(-1,0)=-π/2");
    _F8fly_math5atan2_d_d_d((void*)0, &y4, &x4, &r); check(approx(r,  FLY_PI / 4.0),"atan2(1,1)=π/4");
}

static void test_sqrt(void) {
    double r;
    double v0 = 0.0, v1 = 1.0, v4 = 4.0, v9 = 9.0;

    _F8fly_math4sqrt_d_d((void*)0, &v0, &r); check(approx(r, 0.0), "sqrt(0)");
    _F8fly_math4sqrt_d_d((void*)0, &v1, &r); check(approx(r, 1.0), "sqrt(1)");
    _F8fly_math4sqrt_d_d((void*)0, &v4, &r); check(approx(r, 2.0), "sqrt(4)");
    _F8fly_math4sqrt_d_d((void*)0, &v9, &r); check(approx(r, 3.0), "sqrt(9)");
}

static void test_pow(void) {
    double r;
    double b2 = 2.0, e10 = 10.0, b3 = 3.0, e3 = 3.0;

    _F8fly_math3pow_d_d_d((void*)0, &b2, &e10, &r); check(approx(r, 1024.0), "2^10=1024");
    _F8fly_math3pow_d_d_d((void*)0, &b3, &e3,  &r); check(approx(r, 27.0),   "3^3=27");
}

static void test_log_exp(void) {
    double r, r2, one = 1.0;
    double eps = 1e-10;
    int ok;

    double vals[3] = {0.5, 1.0, 7.389056};
    for (int i = 0; i < 3; i++) {
        _F8fly_math3log_d_d((void*)0, &vals[i], &r);
        _F8fly_math3exp_d_d((void*)0, &r, &r2);
        _F8fly_math11approxEqual_d_d_d_b((void*)0, &r2, &vals[i], &eps, &ok);
        check(ok != 0, "exp(log(x))==x");
    }

    double e = FLY_E;
    _F8fly_math3log_d_d((void*)0, &e, &r);
    _F8fly_math11approxEqual_d_d_d_b((void*)0, &r, &one, &eps, &ok);
    check(ok != 0, "log(e)==1");
}

static void test_rounding(void) {
    double r;
    double neg25 = -2.5, pos25 = 2.5, neg17 = -1.7, pos17 = 1.7;

    _F8fly_math5floor_d_d((void*)0, &neg25, &r); check(approx(r, -3.0), "floor(-2.5)");
    _F8fly_math5floor_d_d((void*)0, &pos25, &r); check(approx(r,  2.0), "floor(2.5)");
    _F8fly_math4ceil_d_d ((void*)0, &neg25, &r); check(approx(r, -2.0), "ceil(-2.5)");
    _F8fly_math4ceil_d_d ((void*)0, &pos25, &r); check(approx(r,  3.0), "ceil(2.5)");
    _F8fly_math5round_d_d((void*)0, &neg25, &r); check(approx(r, -3.0), "round(-2.5)");
    _F8fly_math5round_d_d((void*)0, &pos25, &r); check(approx(r,  3.0), "round(2.5)");
    _F8fly_math5trunc_d_d((void*)0, &neg17, &r); check(approx(r, -1.0), "trunc(-1.7)");
    _F8fly_math5trunc_d_d((void*)0, &pos17, &r); check(approx(r,  1.0), "trunc(1.7)");
}

static void test_round_to(void) {
    double r;
    double pi = 3.14159;
    int n2 = 2;
    _F8fly_math7roundTo_d_i_d((void*)0, &pi, &n2, &r);
    check(approx(r, 3.14), "roundTo(3.14159, 2)");
}

static void test_classification(void) {
    double nan_v = FLY_NAN, inf_v = FLY_INF, one = 1.0;
    int r;

    _F8fly_math5isNaN_d_b   ((void*)0, &nan_v, &r); check(r != 0, "isNaN(NaN)");
    _F8fly_math5isNaN_d_b   ((void*)0, &one,   &r); check(r == 0, "isNaN(1)");
    _F8fly_math5isInf_d_b   ((void*)0, &inf_v, &r); check(r != 0, "isInf(Inf)");
    _F8fly_math5isInf_d_b   ((void*)0, &one,   &r); check(r == 0, "isInf(1)");
    _F8fly_math8isFinite_d_b((void*)0, &one,   &r); check(r != 0, "isFinite(1)");
    _F8fly_math8isFinite_d_b((void*)0, &inf_v, &r); check(r == 0, "isFinite(Inf)");
    _F8fly_math8isFinite_d_b((void*)0, &nan_v, &r); check(r == 0, "isFinite(NaN)");
}

static void test_approx_equal(void) {
    double a = 1.0, b = 1.0 + FLY_EPSILON * 0.5;
    double c = 1.0 + FLY_EPSILON * 10.0;
    double eps = FLY_EPSILON;
    int r;

    _F8fly_math11approxEqual_d_d_d_b((void*)0, &a, &b, &eps, &r); check(r != 0, "approxEq within epsilon");
    _F8fly_math11approxEqual_d_d_d_b((void*)0, &a, &c, &eps, &r); check(r == 0, "approxEq outside epsilon");
}

static void test_ispow2_nextpow2(void) {
    int r;
    int64_t v;
    int64_t n0 = 0, n1 = 1, n2 = 2, n3 = 3, n4 = 4, n1024 = 1024;

    _F8fly_math6isPow2_l_b((void*)0, &n0,    &r); check(r == 0, "isPow2(0)");
    _F8fly_math6isPow2_l_b((void*)0, &n1,    &r); check(r != 0, "isPow2(1)");
    _F8fly_math6isPow2_l_b((void*)0, &n2,    &r); check(r != 0, "isPow2(2)");
    _F8fly_math6isPow2_l_b((void*)0, &n3,    &r); check(r == 0, "isPow2(3)");
    _F8fly_math6isPow2_l_b((void*)0, &n4,    &r); check(r != 0, "isPow2(4)");
    _F8fly_math6isPow2_l_b((void*)0, &n1024, &r); check(r != 0, "isPow2(1024)");

    int64_t n5 = 5, n8 = 8, n100 = 100;
    _F8fly_math8nextPow2_l_l((void*)0, &n1,   &v); check(v == 1,   "nextPow2(1)");
    _F8fly_math8nextPow2_l_l((void*)0, &n3,   &v); check(v == 4,   "nextPow2(3)");
    _F8fly_math8nextPow2_l_l((void*)0, &n5,   &v); check(v == 8,   "nextPow2(5)");
    _F8fly_math8nextPow2_l_l((void*)0, &n8,   &v); check(v == 8,   "nextPow2(8)");
    _F8fly_math8nextPow2_l_l((void*)0, &n100, &v); check(v == 128, "nextPow2(100)");
}

static void test_popcount(void) {
    int r;
    int64_t n0 = 0, n1 = 1, n255 = 255, nmax = FLY_MAX_INT64;

    _F8fly_math8popcount_l_i((void*)0, &n0,   &r); check(r == 0,  "popcount(0)");
    _F8fly_math8popcount_l_i((void*)0, &n1,   &r); check(r == 1,  "popcount(1)");
    _F8fly_math8popcount_l_i((void*)0, &n255, &r); check(r == 8,  "popcount(255)");
    _F8fly_math8popcount_l_i((void*)0, &nmax, &r); check(r == 63, "popcount(INT64_MAX)");
}

static void test_leading_trailing_zeros(void) {
    int r;
    int64_t n0 = 0, n1 = 1, n2 = 2, n4 = 4, n8 = 8;

    _F8fly_math12leadingZeros_l_i((void*)0, &n1, &r); check(r == 63, "clz(1)");
    _F8fly_math12leadingZeros_l_i((void*)0, &n2, &r); check(r == 62, "clz(2)");
    _F8fly_math12leadingZeros_l_i((void*)0, &n0, &r); check(r == 64, "clz(0)");

    _F8fly_math13trailingZeros_l_i((void*)0, &n1, &r); check(r == 0,  "ctz(1)");
    _F8fly_math13trailingZeros_l_i((void*)0, &n4, &r); check(r == 2,  "ctz(4)");
    _F8fly_math13trailingZeros_l_i((void*)0, &n8, &r); check(r == 3,  "ctz(8)");
    _F8fly_math13trailingZeros_l_i((void*)0, &n0, &r); check(r == 64, "ctz(0)");
}

static void test_rand(void) {
    fly_rng rng;
    uint64_t seed = 42ULL;
    int64_t lo = 0, hi = 99;
    _F8fly_math8randSeed_Cfly_rng_ul((void*)0, &rng, &seed);

    int range_ok = 1;
    for (int i = 0; i < 1000; i++) {
        int64_t v;
        _F8fly_math7randInt_Cfly_rng_l_l_l((void*)0, &rng, &lo, &hi, &v);
        if (v < lo || v > hi) { range_ok = 0; break; }
    }
    check(range_ok, "randInt range [0,99]");

    int float_ok = 1;
    for (int i = 0; i < 1000; i++) {
        double v;
        _F8fly_math9randFloat_Cfly_rng_d((void*)0, &rng, &v);
        if (v < 0.0 || v >= 1.0) { float_ok = 0; break; }
    }
    check(float_ok, "randFloat range [0,1)");
}

static void test_fma(void) {
    double r;
    double a = 2.0, b = 3.0, c = 4.0;
    _F8fly_math3fma_d_d_d_d((void*)0, &a, &b, &c, &r);
    check(approx(r, 10.0), "fma(2,3,4)=10");
}

static void test_lerp(void) {
    double r;
    double a = 0.0, b = 10.0, t0 = 0.0, t1 = 1.0, t5 = 0.5;

    _F8fly_math4lerp_d_d_d_d((void*)0, &a, &b, &t0, &r); check(approx(r,  0.0), "lerp t=0 → a");
    _F8fly_math4lerp_d_d_d_d((void*)0, &a, &b, &t1, &r); check(approx(r, 10.0), "lerp t=1 → b");
    _F8fly_math4lerp_d_d_d_d((void*)0, &a, &b, &t5, &r); check(approx(r,  5.0), "lerp t=0.5 → mid");
}

static void test_saturate(void) {
    double r;
    double neg = -1.0, zer = 0.0, mid = 0.5, one = 1.0, above = 2.0;

    _F8fly_math8saturate_d_d((void*)0, &neg,   &r); check(approx(r, 0.0), "saturate(-1)");
    _F8fly_math8saturate_d_d((void*)0, &zer,   &r); check(approx(r, 0.0), "saturate(0)");
    _F8fly_math8saturate_d_d((void*)0, &mid,   &r); check(approx(r, 0.5), "saturate(0.5)");
    _F8fly_math8saturate_d_d((void*)0, &one,   &r); check(approx(r, 1.0), "saturate(1)");
    _F8fly_math8saturate_d_d((void*)0, &above, &r); check(approx(r, 1.0), "saturate(2)");
}

static void test_factorial(void) {
    int64_t ri; double rd;
    int n0 = 0, n1 = 1, n5 = 5, n10 = 10;

    _F8fly_math10factorialI_i_l((void*)0, &n0,  &ri); check(ri == 1,       "0!=1");
    _F8fly_math10factorialI_i_l((void*)0, &n1,  &ri); check(ri == 1,       "1!=1");
    _F8fly_math10factorialI_i_l((void*)0, &n5,  &ri); check(ri == 120,     "5!=120");
    _F8fly_math10factorialI_i_l((void*)0, &n10, &ri); check(ri == 3628800, "10!=3628800");

    _F8fly_math10factorialF_i_d((void*)0, &n5, &rd); check(approx(rd, 120.0), "5!F=120");
}

static void test_comb(void) {
    int64_t r;
    int64_t n5 = 5, k2 = 2, n10 = 10, k3 = 3;

    _F8fly_math4comb_l_l_l((void*)0, &n5,  &k2, &r); check(r == 10,  "C(5,2)=10");
    _F8fly_math4comb_l_l_l((void*)0, &n10, &k3, &r); check(r == 120, "C(10,3)=120");
}

static void test_gamma(void) {
    double r, one = 1.0, half = 0.5;
    double eps = 1e-7;
    int ok;

    _F8fly_math5gamma_d_d((void*)0, &one, &r);
    _F8fly_math11approxEqual_d_d_d_b((void*)0, &r, &one, &eps, &ok);
    check(ok != 0, "Γ(1)=1");

    double sqrt_pi = __builtin_sqrt(FLY_PI);
    _F8fly_math5gamma_d_d((void*)0, &half, &r);
    _F8fly_math11approxEqual_d_d_d_b((void*)0, &r, &sqrt_pi, &eps, &ok);
    check(ok != 0, "Γ(0.5)=√π");
}

/* Align the stack to 16 bytes before calling into math functions.
 * The Linux kernel enters _start with rsp not guaranteed 16-byte aligned;
 * mathRandNormal uses movapd which requires 16-byte alignment. */
static void run_tests(void);

__attribute__((naked)) void _start(void) {
    __asm__ volatile(
        "andq  $-16, %%rsp\n"
        "callq %P0\n"
        : : "i"(run_tests)
    );
}

static void run_tests(void) {
    test_abs();
    test_sign();
    test_clamp();
    test_gcd_lcm();
    test_sin_cos();
    test_atan2();
    test_sqrt();
    test_pow();
    test_log_exp();
    test_rounding();
    test_round_to();
    test_classification();
    test_approx_equal();
    test_ispow2_nextpow2();
    test_popcount();
    test_leading_trailing_zeros();
    test_rand();
    test_fma();
    test_lerp();
    test_saturate();
    test_factorial();
    test_comb();
    test_gamma();

    uint64_t total = (uint64_t)(g_pass + g_fail);
    fly_puts("fly_math: ");
    fly_put_u64((uint64_t)g_pass);
    fly_write("/", 1);
    fly_put_u64(total);
    fly_puts(" passed");
    if (g_fail) {
        fly_puts("  (");
        fly_put_u64((uint64_t)g_fail);
        fly_puts(" FAILED)");
    }
    fly_write("\n", 1);
    fly_exit(g_fail ? 1 : 0);
}
