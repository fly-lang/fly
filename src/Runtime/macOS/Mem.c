/*===-- src/Runtime/macOS/Mem.c - Memory via mmap/munmap ------------------===*/

#include "Runtime/Runtime.h"
#include "LibSystem.h"

void *mem_alloc(usize size)
{
    void *p = mmap((void *)0, (size_rt)size,
                   PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0LL);
    return (p == MAP_FAILED) ? (void *)0 : p;
}

i32 mem_free(void *ptr, usize size)
{
    return (munmap(ptr, (size_rt)size) == 0) ? 0 : -1;
}
