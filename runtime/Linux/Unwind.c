// Stub implementations of the Itanium EH unwinding API.
// Fly does not emit exception-throwing code, so these functions are never
// called at runtime.  They exist solely to satisfy linker references that
// come from libc.a cold paths and compiler-rt's gcc_personality_v0 object.

#include "../Runtime.h"

// __SIZE_TYPE__ is pointer-sized and always available in freestanding mode.
typedef __SIZE_TYPE__ _fly_uptr;

FLY_NORETURN
void _Unwind_Resume(void *exc) { __builtin_trap(); }

_fly_uptr _Unwind_GetLanguageSpecificData(void *ctx) { (void)ctx; return 0; }
_fly_uptr _Unwind_GetIP(void *ctx)                   { (void)ctx; return 0; }
_fly_uptr _Unwind_GetRegionStart(void *ctx)          { (void)ctx; return 0; }
void      _Unwind_SetGR(void *ctx, int col, _fly_uptr val) { (void)ctx; (void)col; (void)val; }
void      _Unwind_SetIP(void *ctx, _fly_uptr val)         { (void)ctx; (void)val; }
