/*===-- runtime/Linux/Mem.c - Memory allocation via mmap/munmap -------===*/

#include "../Runtime.h"
#include "Syscall.h"

#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define MAP_PRIVATE 0x2
#define MAP_ANON    0x20

void *mem_alloc(usize size)
{
    long ret = __syscall6(SYS_mmap, 0L, (long)size,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANON,
                          -1L, 0L);
    if (ret < 0 && ret > -4096L)
        return (void *)0;
    return (void *)ret;
}

i32 mem_free(void *ptr, usize size)
{
    long ret = __syscall2(SYS_munmap, (long)ptr, (long)size);
    return (ret == 0) ? 0 : -1;
}
