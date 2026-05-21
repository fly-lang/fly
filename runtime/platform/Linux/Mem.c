/*===-- runtime/Linux/Mem.c - Memory allocation via malloc/free --------===*/

#include "../Runtime.h"

/* Forward-declare libc malloc/free without pulling in any libc header.
 * All Fly programs link against libc (the compiler emits call @malloc for
 * string initialisation), so these symbols are always available at link time.
 * Using malloc ensures mem_alloc-returned pointers are compatible with free(),
 * which the compiler emits in EmitAllocCleanup for string variable cleanup. */
extern void *malloc(usize size);
extern void  free(void *ptr);

void *mem_alloc(usize size)
{
    return malloc(size > 0 ? size : 1);
}

i32 mem_free(void *ptr, usize size)
{
    (void)size;
    free(ptr);
    return 0;
}
