; std/math/fly_math.ll — Level-2 LLVM IR overrides for fly.math
;
; This file holds functions that cannot emit the expected LLVM intrinsic
; via C/__builtin_* alone (verified with clang -S -emit-llvm -O2).
;
; Add a function here ONLY when ALL of the following are true:
;   1. clang -S -emit-llvm -O2 -fno-math-errno -fno-trapping-math fly_math.c
;      confirms the expected intrinsic is NOT emitted.
;   2. A custom calling convention or SIMD sequence is needed that C cannot
;      express.
;   3. The function is a 2-3 instruction wrapper where C overhead has cost.
;
; ── Functions documented as libm calls (no LLVM 20 intrinsic available) ─────
;
; mathErf / mathGamma
;   Verified:
;     clang -S -emit-llvm -O2 -fno-math-errno -fno-trapping-math
;           -nostdinc fly_math.c -I. | grep -E "erf|tgamma"
;   Result: tail call double @erf(...) and @tgamma(...)
;   LLVM 20 has no llvm.erf.f64 or llvm.tgamma.f64 intrinsics.
;   These remain as libm calls at link time (satisfied by -lm in tests).
;   Future: polynomial approx (Abramowitz & Stegun for erf;
;           Lanczos for gamma) for fully freestanding operation.
;
; ── Level-2 overrides (none yet) ────────────────────────────────────────────
;
; Example skeleton:
;
; ; mathExampleFn — __builtin_X does not emit llvm.X.f64
; ; Verified: clang -S -emit-llvm -O2 -fno-math-errno -fno-trapping-math
; ;           -nostdinc fly_math.c -I. | grep llvm.X  (not found)
; define void @mathExampleFn(ptr %x, ptr %out) {
;   %v = load double, ptr %x
;   %r = call double @llvm.exampleFn.f64(double %v)
;   store double %r, ptr %out
;   ret void
; }
