/*===-- std/math/fly_math.c - fly.math native implementations -----------===
 *
 * All functions use Clang __builtin_* which Clang lowers to LLVM intrinsics.
 * -fno-math-errno -fno-trapping-math are required to emit intrinsics for
 * transcendentals (sin, cos, sqrt, exp, …) rather than libm calls.
 *
 * Verify with:
 *   clang -S -emit-llvm -O2 -nostdlib -nostdinc \
 *         -fno-math-errno -fno-trapping-math fly_math.c -o fly_math.ll
 *   grep "llvm\." fly_math.ll
 *
 * Level-2 IR (.ll) overrides live in fly_math.ll; see that file for the
 * list of functions moved there and the reason in each case.
 *===----------------------------------------------------------------------===*/

#include "fly_math.h"

/* ══════════════════════════════════════════════════════════════════════════ */
/* Base                                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math4absI_l_l(void *err_ctx, const int64_t *x, int64_t *out) {
    (void)err_ctx;
    int64_t v = *x;
    *out = v < 0 ? -v : v;
}

void _F8fly_math4absF_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_fabs(*x);
}

void _F8fly_math4sign_d_i(void *err_ctx, const double *x, int *out) {
    (void)err_ctx;
    double v = *x;
    *out = (int)((v > 0.0) - (v < 0.0));
}

void _F8fly_math4minI_l_l_l(void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out) {
    (void)err_ctx;
    *out = *a < *b ? *a : *b;
}

void _F8fly_math4minF_d_d_d(void *err_ctx, const double *a, const double *b, double *out) {
    (void)err_ctx;
    *out = __builtin_fmin(*a, *b);
}

void _F8fly_math4maxI_l_l_l(void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out) {
    (void)err_ctx;
    *out = *a > *b ? *a : *b;
}

void _F8fly_math4maxF_d_d_d(void *err_ctx, const double *a, const double *b, double *out) {
    (void)err_ctx;
    *out = __builtin_fmax(*a, *b);
}

void _F8fly_math6clampI_l_l_l_l(void *err_ctx, const int64_t *x, const int64_t *lo, const int64_t *hi, int64_t *out) {
    (void)err_ctx;
    int64_t v = *x;
    if (v < *lo) v = *lo;
    if (v > *hi) v = *hi;
    *out = v;
}

void _F8fly_math6clampF_d_d_d_d(void *err_ctx, const double *x, const double *lo, const double *hi, double *out) {
    (void)err_ctx;
    double v = *x;
    v = __builtin_fmax(v, *lo);
    v = __builtin_fmin(v, *hi);
    *out = v;
}

void _F8fly_math3gcd_l_l_l(void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out) {
    (void)err_ctx;
    int64_t u = *a < 0 ? -*a : *a;
    int64_t v = *b < 0 ? -*b : *b;
    while (v) {
        int64_t t = v;
        v = u % v;
        u = t;
    }
    *out = u;
}

void _F8fly_math3lcm_l_l_l(void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out) {
    int64_t g;
    _F8fly_math3gcd_l_l_l(err_ctx, a, b, &g);
    if (g == 0) { *out = 0; return; }
    int64_t av = *a < 0 ? -*a : *a;
    int64_t bv = *b < 0 ? -*b : *b;
    *out = (av / g) * bv;
}

void _F8fly_math6divmod_l_l_l_l(void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out_q, int64_t *out_r) {
    (void)err_ctx;
    *out_q = *a / *b;
    *out_r = *a % *b;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Trigonometry                                                               */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math3sin_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_sin(*x);
}

void _F8fly_math3cos_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_cos(*x);
}

void _F8fly_math3tan_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_tan(*x);
}

void _F8fly_math4asin_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_asin(*x);
}

void _F8fly_math4acos_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_acos(*x);
}

void _F8fly_math4atan_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_atan(*x);
}

void _F8fly_math5atan2_d_d_d(void *err_ctx, const double *y, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_atan2(*y, *x);
}

void _F8fly_math4sinh_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_sinh(*x);
}

void _F8fly_math4cosh_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_cosh(*x);
}

void _F8fly_math4tanh_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_tanh(*x);
}

void _F8fly_math9toRadians_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = *x * (FLY_PI / 180.0);
}

void _F8fly_math9toDegrees_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = *x * (180.0 / FLY_PI);
}

void _F8fly_math5hypot_d_d_d(void *err_ctx, const double *a, const double *b, double *out) {
    (void)err_ctx;
    /* __builtin_hypot emits a @hypot libm call; use sqrt(a²+b²) directly
     * to get llvm.sqrt.f64.  -ffast-math is acceptable here per spec. */
    *out = __builtin_sqrt(*a * *a + *b * *b);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Exponential & Logarithms                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math4sqrt_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_sqrt(*x);
}

void _F8fly_math4cbrt_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    /* __builtin_cbrt does not emit llvm.cbrt.f64 in LLVM 20 (verified).
     * Decompose via pow + sign to get llvm.pow.f64 + llvm.fabs.f64 instead. */
    double v = *x;
    double abs_v = __builtin_fabs(v);
    double r = __builtin_pow(abs_v, 1.0 / 3.0);
    *out = v < 0.0 ? -r : r;
}

void _F8fly_math3pow_d_d_d(void *err_ctx, const double *base, const double *exp, double *out) {
    (void)err_ctx;
    *out = __builtin_pow(*base, *exp);
}

void _F8fly_math3exp_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_exp(*x);
}

void _F8fly_math4exp2_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_exp2(*x);
}

void _F8fly_math3log_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_log(*x);
}

void _F8fly_math4log2_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_log2(*x);
}

void _F8fly_math5log10_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_log10(*x);
}

void _F8fly_math4logN_d_d_d(void *err_ctx, const double *x, const double *n, double *out) {
    (void)err_ctx;
    *out = __builtin_log(*x) / __builtin_log(*n);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Rounding                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math5floor_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_floor(*x);
}

void _F8fly_math4ceil_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_ceil(*x);
}

void _F8fly_math5round_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_round(*x);
}

void _F8fly_math5trunc_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_trunc(*x);
}

void _F8fly_math7roundTo_d_i_d(void *err_ctx, const double *x, const int *n, double *out) {
    (void)err_ctx;
    double factor = 1.0;
    int k = *n;
    if (k >= 0) {
        for (int i = 0; i < k; i++) factor *= 10.0;
        *out = __builtin_round(*x * factor) / factor;
    } else {
        for (int i = 0; i > k; i--) factor *= 10.0;
        *out = __builtin_round(*x / factor) * factor;
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Classification                                                             */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math5isNaN_d_b(void *err_ctx, const double *x, int *out) {
    (void)err_ctx;
    *out = __builtin_isnan(*x);
}

void _F8fly_math5isInf_d_b(void *err_ctx, const double *x, int *out) {
    (void)err_ctx;
    *out = __builtin_isinf(*x);
}

void _F8fly_math8isFinite_d_b(void *err_ctx, const double *x, int *out) {
    (void)err_ctx;
    *out = __builtin_isfinite(*x);
}

void _F8fly_math11approxEqual_d_d_d_b(void *err_ctx, const double *a, const double *b, const double *epsilon, int *out) {
    (void)err_ctx;
    double diff = *a - *b;
    if (diff < 0.0) diff = -diff;
    *out = diff <= *epsilon;
}

void _F8fly_math8copySign_d_d_d(void *err_ctx, const double *x, const double *y, double *out) {
    (void)err_ctx;
    *out = __builtin_copysign(*x, *y);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Bitwise (long / int64)                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math6isPow2_l_b(void *err_ctx, const int64_t *n, int *out) {
    (void)err_ctx;
    int64_t v = *n;
    *out = (v > 0) && ((v & (v - 1)) == 0);
}

void _F8fly_math8nextPow2_l_l(void *err_ctx, const int64_t *n, int64_t *out) {
    (void)err_ctx;
    int64_t v = *n;
    if (v <= 1) { *out = 1; return; }
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    *out = v + 1;
}

void _F8fly_math8popcount_l_i(void *err_ctx, const int64_t *n, int *out) {
    (void)err_ctx;
    *out = __builtin_popcountll((unsigned long long)*n);
}

void _F8fly_math12leadingZeros_l_i(void *err_ctx, const int64_t *n, int *out) {
    (void)err_ctx;
    unsigned long long v = (unsigned long long)*n;
    *out = v == 0 ? 64 : __builtin_clzll(v);
}

void _F8fly_math13trailingZeros_l_i(void *err_ctx, const int64_t *n, int *out) {
    (void)err_ctx;
    unsigned long long v = (unsigned long long)*n;
    *out = v == 0 ? 64 : __builtin_ctzll(v);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Random (xoshiro256**)                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

static uint64_t xoshiro_rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoshiro_next(fly_rng *rng) {
    uint64_t result = xoshiro_rotl(rng->s[1] * 5, 7) * 9;
    uint64_t t = rng->s[1] << 17;
    rng->s[2] ^= rng->s[0];
    rng->s[3] ^= rng->s[1];
    rng->s[1] ^= rng->s[2];
    rng->s[0] ^= rng->s[3];
    rng->s[2] ^= t;
    rng->s[3] = xoshiro_rotl(rng->s[3], 45);
    return result;
}

/* splitmix64 seeding: one pass per state word */
void _F8fly_math8randSeed_Cfly_rng_ul(void *err_ctx, fly_rng *rng, const uint64_t *seed) {
    (void)err_ctx;
    uint64_t s = *seed;
    for (int i = 0; i < 4; i++) {
        s += 0x9e3779b97f4a7c15ULL;
        uint64_t z = s;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        rng->s[i] = z ^ (z >> 31);
    }
}

void _F8fly_math9randFloat_Cfly_rng_d(void *err_ctx, fly_rng *rng, double *out) {
    (void)err_ctx;
    /* top 53 bits → mantissa of a double in [0, 1) */
    uint64_t r = xoshiro_next(rng);
    *out = (double)(r >> 11) * (1.0 / (double)(1ULL << 53));
}

void _F8fly_math7randInt_Cfly_rng_l_l_l(void *err_ctx, fly_rng *rng, const int64_t *a, const int64_t *b, int64_t *out) {
    (void)err_ctx;
    int64_t lo = *a, hi = *b;
    if (lo > hi) { *out = lo; return; }
    uint64_t range = (uint64_t)(hi - lo) + 1;
    if (range == 0) { /* full 64-bit range */
        *out = (int64_t)xoshiro_next(rng);
        return;
    }
    /* Debiased rejection sampling — Lemire 2019 */
    uint64_t threshold = ((uint64_t)(-(int64_t)range)) % range;
    uint64_t r;
    do { r = xoshiro_next(rng); } while (r < threshold);
    *out = lo + (int64_t)(r % range);
}

void _F8fly_math10randNormal_Cfly_rng_d_d_d(void *err_ctx, fly_rng *rng, const double *mu, const double *sigma, double *out) {
    double u1, u2;
    _F8fly_math9randFloat_Cfly_rng_d(err_ctx, rng, &u1);
    _F8fly_math9randFloat_Cfly_rng_d(err_ctx, rng, &u2);
    if (u1 <= 0.0) u1 = FLY_MIN_F64;
    double mag = *sigma * __builtin_sqrt(-2.0 * __builtin_log(u1));
    *out = mag * __builtin_cos(2.0 * FLY_PI * u2) + *mu;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Special Functions                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math3fma_d_d_d_d(void *err_ctx, const double *a, const double *b, const double *c, double *out) {
    (void)err_ctx;
    *out = __builtin_fma(*a, *b, *c);
}

void _F8fly_math4lerp_d_d_d_d(void *err_ctx, const double *a, const double *b, const double *t, double *out) {
    (void)err_ctx;
    /* Precise at endpoints: lerp(a,b,0)==a and lerp(a,b,1)==b exactly */
    double tv = *t;
    *out = *a * (1.0 - tv) + *b * tv;
}

void _F8fly_math8saturate_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    double v = *x;
    *out = v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
}

void _F8fly_math10factorialI_i_l(void *err_ctx, const int *n, int64_t *out) {
    (void)err_ctx;
    int k = *n;
    int64_t r = 1;
    for (int i = 2; i <= k; i++) r *= (int64_t)i;
    *out = r;
}

void _F8fly_math10factorialF_i_d(void *err_ctx, const int *n, double *out) {
    (void)err_ctx;
    int k = *n;
    double r = 1.0;
    for (int i = 2; i <= k; i++) r *= (double)i;
    *out = r;
}

void _F8fly_math4comb_l_l_l(void *err_ctx, const int64_t *n, const int64_t *k, int64_t *out) {
    (void)err_ctx;
    int64_t nv = *n, kv = *k;
    if (kv < 0 || kv > nv) { *out = 0; return; }
    if (kv > nv - kv) kv = nv - kv;   /* C(n,k) == C(n,n-k) */
    int64_t r = 1;
    for (int64_t i = 0; i < kv; i++)
        r = r * (nv - i) / (i + 1);
    *out = r;
}

void _F8fly_math4perm_l_l_l(void *err_ctx, const int64_t *n, const int64_t *k, int64_t *out) {
    (void)err_ctx;
    int64_t nv = *n, kv = *k;
    if (kv < 0 || kv > nv) { *out = 0; return; }
    int64_t r = 1;
    for (int64_t i = 0; i < kv; i++) r *= (nv - i);
    *out = r;
}

void _F8fly_math3erf_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_erf(*x);
}

void _F8fly_math5gamma_d_d(void *err_ctx, const double *x, double *out) {
    (void)err_ctx;
    *out = __builtin_tgamma(*x);
}
