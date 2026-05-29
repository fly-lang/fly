/*===-- runtime/macOS/Mem.c - Memory via malloc/free ------------------===*/

#include "../Runtime.h"

/* Forward-declare libc malloc/free without pulling in any libc header.
 * All Fly programs link against libc, so these are always available at link
 * time. Using malloc ensures compatibility with free() in EmitAllocCleanup. */
extern void *malloc(usize size);
extern void  free(void *ptr);
extern void *realloc(void *ptr, usize size);

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

void *mem_realloc(void *ptr, usize new_size)
{
    return realloc(ptr, new_size > 0 ? new_size : 1);
}
