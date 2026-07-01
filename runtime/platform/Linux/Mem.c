/*===-- runtime/Linux/Mem.c - Memory allocation via malloc/free --------===*/

#include "../Runtime.h"

/* Forward-declare libc malloc/free without pulling in any libc header.
 * All Fly programs link against libc (the compiler emits call @malloc for
 * string initialisation), so these symbols are always available at link time.
 * Using malloc ensures mem_alloc-returned pointers are compatible with free(),
 * which the compiler emits in EmitAllocCleanup for string variable cleanup. */
extern void *malloc(usize size);
extern void  free(void *ptr);
extern void *realloc(void *ptr, usize size);
extern void *calloc(usize nmemb, usize size);

/* mem_alloc returns UNINITIALISED memory (Rust's GlobalAlloc::alloc / malloc).
 * Callers that need zeroed memory use mem_alloc_zeroed. Object construction is
 * made deterministic by the compiler, which memset-zeroes each `new` allocation
 * (see CodeGenExpr) rather than relying on the allocator. */
void *mem_alloc(usize size)
{
    return malloc(size > 0 ? size : 1);
}

/* mem_alloc_zeroed returns ZEROED memory (Rust's GlobalAlloc::alloc_zeroed = calloc). */
void *mem_alloc_zeroed(usize size)
{
    return calloc(1, size > 0 ? size : 1);
}

i32 mem_free(void *ptr, usize size)
{
    (void)size;
    free(ptr);
    return 0;
}

void *mem_realloc(void *ptr, usize new_size)
{
    return realloc(ptr, new_size > 0 ? new_size : 1);
}
