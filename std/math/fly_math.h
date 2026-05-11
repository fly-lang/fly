/*===-- std/math/fly_math.h - fly.math C types and declarations ---------===
 *
 * Internal header shared between fly_math.c and fly_math_test.c.
 * No libc headers — only fundamental C types.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_MATH_H
#define FLY_MATH_H

/* ── Portable integer types (no system headers) ───────────────────────────── */
typedef __INT64_TYPE__   int64_t;
typedef __UINT64_TYPE__  uint64_t;
typedef __INT32_TYPE__   int32_t;

/* ── Mathematical constants ───────────────────────────────────────────────── */
#define FLY_PI       3.14159265358979323846
#define FLY_E        2.71828182845904523536
#define FLY_SQRT2    1.41421356237309504880
#define FLY_LN2      0.69314718055994530941
#define FLY_INF      __builtin_inf()
#define FLY_NAN      __builtin_nan("")
#define FLY_EPSILON  2.2204460492503131e-16
#define FLY_MAX_INT64   9223372036854775807LL
#define FLY_MIN_INT64  (-9223372036854775807LL - 1)
#define FLY_MAX_F64    1.7976931348623157e+308
#define FLY_MIN_F64    2.2250738585072014e-308

/* ── Random generator state (xoshiro256**) ───────────────────────────────── */
typedef struct { uint64_t s[4]; } fly_rng;

/* ── Calling convention ──────────────────────────────────────────────────── *
 *  All Fly functions receive:
 *    arg 0 : void *err_ctx  — implicit error-handler context (unused here)
 *    arg N : T *param       — each explicit param passed by reference
 *  All functions return void; scalar results are written to an output param.
 *
 *  Name mangling: _F<ns_len><ns_flat><fn_len><fn><param-type-codes>
 *    namespace fly.math → ns_flat = "fly_math" (8 chars)
 *    bool → _b   byte → _y   short → _s    int  → _i
 *    ulong → _ul  long → _l   float → _f    double → _d
 *    string → _Ss  error → _e   array → _A<elem>  class → _C<name>
 *
 *  Note: bool output params use int* in C (Fly bool is represented as int).
 *===----------------------------------------------------------------------=== */

/* ══════════════════════════════════════════════════════════════════════════ */
/* Base                                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math4absI_l_l       (void *err_ctx, const int64_t *x, int64_t *out);
void _F8fly_math4absF_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math4sign_d_i       (void *err_ctx, const double *x, int *out);
void _F8fly_math4minI_l_l_l     (void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out);
void _F8fly_math4minF_d_d_d     (void *err_ctx, const double *a,  const double *b,  double *out);
void _F8fly_math4maxI_l_l_l     (void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out);
void _F8fly_math4maxF_d_d_d     (void *err_ctx, const double *a,  const double *b,  double *out);
void _F8fly_math6clampI_l_l_l_l (void *err_ctx, const int64_t *x, const int64_t *lo, const int64_t *hi, int64_t *out);
void _F8fly_math6clampF_d_d_d_d (void *err_ctx, const double *x,  const double *lo,  const double *hi,  double *out);
void _F8fly_math3gcd_l_l_l      (void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out);
void _F8fly_math3lcm_l_l_l      (void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out);
void _F8fly_math6divmod_l_l_l_l (void *err_ctx, const int64_t *a, const int64_t *b, int64_t *out_q, int64_t *out_r);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Trigonometry                                                               */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math3sin_d_d        (void *err_ctx, const double *x, double *out);   /* llvm.sin.f64  */
void _F8fly_math3cos_d_d        (void *err_ctx, const double *x, double *out);   /* llvm.cos.f64  */
void _F8fly_math3tan_d_d        (void *err_ctx, const double *x, double *out);   /* llvm.tan.f64  */
void _F8fly_math4asin_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math4acos_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math4atan_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math5atan2_d_d_d    (void *err_ctx, const double *y, const double *x, double *out);
void _F8fly_math4sinh_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math4cosh_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math4tanh_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math9toRadians_d_d  (void *err_ctx, const double *x, double *out);
void _F8fly_math9toDegrees_d_d  (void *err_ctx, const double *x, double *out);
void _F8fly_math5hypot_d_d_d    (void *err_ctx, const double *a, const double *b, double *out);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Exponential & Logarithms                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math4sqrt_d_d       (void *err_ctx, const double *x, double *out);   /* llvm.sqrt.f64  */
void _F8fly_math4cbrt_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math3pow_d_d_d      (void *err_ctx, const double *base, const double *exp, double *out); /* llvm.pow.f64 */
void _F8fly_math3exp_d_d        (void *err_ctx, const double *x, double *out);   /* llvm.exp.f64   */
void _F8fly_math4exp2_d_d       (void *err_ctx, const double *x, double *out);   /* llvm.exp2.f64  */
void _F8fly_math3log_d_d        (void *err_ctx, const double *x, double *out);   /* llvm.log.f64   */
void _F8fly_math4log2_d_d       (void *err_ctx, const double *x, double *out);   /* llvm.log2.f64  */
void _F8fly_math5log10_d_d      (void *err_ctx, const double *x, double *out);   /* llvm.log10.f64 */
void _F8fly_math4logN_d_d_d     (void *err_ctx, const double *x, const double *n, double *out);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Rounding                                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math5floor_d_d      (void *err_ctx, const double *x, double *out);   /* llvm.floor.f64 */
void _F8fly_math4ceil_d_d       (void *err_ctx, const double *x, double *out);   /* llvm.ceil.f64  */
void _F8fly_math5round_d_d      (void *err_ctx, const double *x, double *out);   /* llvm.round.f64 */
void _F8fly_math5trunc_d_d      (void *err_ctx, const double *x, double *out);   /* llvm.trunc.f64 */
void _F8fly_math7roundTo_d_i_d  (void *err_ctx, const double *x, const int *n, double *out);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Classification                                                             */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math5isNaN_d_b          (void *err_ctx, const double *x, int *out);
void _F8fly_math5isInf_d_b          (void *err_ctx, const double *x, int *out);
void _F8fly_math8isFinite_d_b       (void *err_ctx, const double *x, int *out);
void _F8fly_math11approxEqual_d_d_d_b(void *err_ctx, const double *a, const double *b, const double *epsilon, int *out);
void _F8fly_math8copySign_d_d_d     (void *err_ctx, const double *x, const double *y, double *out);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Bitwise (long / int64)                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math6isPow2_l_b         (void *err_ctx, const int64_t *n, int *out);
void _F8fly_math8nextPow2_l_l       (void *err_ctx, const int64_t *n, int64_t *out);
void _F8fly_math8popcount_l_i       (void *err_ctx, const int64_t *n, int *out);   /* llvm.ctpop.i64 */
void _F8fly_math12leadingZeros_l_i  (void *err_ctx, const int64_t *n, int *out);   /* llvm.ctlz.i64  */
void _F8fly_math13trailingZeros_l_i (void *err_ctx, const int64_t *n, int *out);   /* llvm.cttz.i64  */

/* ══════════════════════════════════════════════════════════════════════════ */
/* Random (xoshiro256**)                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math8randSeed_Cfly_rng_ul         (void *err_ctx, fly_rng *rng, const uint64_t *seed);
void _F8fly_math7randInt_Cfly_rng_l_l_l       (void *err_ctx, fly_rng *rng, const int64_t *a, const int64_t *b, int64_t *out);
void _F8fly_math9randFloat_Cfly_rng_d         (void *err_ctx, fly_rng *rng, double *out);
void _F8fly_math10randNormal_Cfly_rng_d_d_d   (void *err_ctx, fly_rng *rng, const double *mu, const double *sigma, double *out);

/* ══════════════════════════════════════════════════════════════════════════ */
/* Special Functions                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F8fly_math3fma_d_d_d_d        (void *err_ctx, const double *a, const double *b, const double *c, double *out); /* llvm.fma.f64 */
void _F8fly_math4lerp_d_d_d_d       (void *err_ctx, const double *a, const double *b, const double *t, double *out);
void _F8fly_math8saturate_d_d       (void *err_ctx, const double *x, double *out);
void _F8fly_math10factorialI_i_l    (void *err_ctx, const int *n, int64_t *out);
void _F8fly_math10factorialF_i_d    (void *err_ctx, const int *n, double *out);
void _F8fly_math4comb_l_l_l         (void *err_ctx, const int64_t *n, const int64_t *k, int64_t *out);
void _F8fly_math4perm_l_l_l         (void *err_ctx, const int64_t *n, const int64_t *k, int64_t *out);
void _F8fly_math3erf_d_d            (void *err_ctx, const double *x, double *out);
void _F8fly_math5gamma_d_d          (void *err_ctx, const double *x, double *out);

#endif /* FLY_MATH_H */
