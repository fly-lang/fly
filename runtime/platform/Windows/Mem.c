/*===-- runtime/Windows/Mem.c - Memory via VirtualAlloc/Free ----------===*/

#include "../Runtime.h"
#include "Win32.h"

void *mem_alloc(usize size)
{
    return VirtualAlloc(WIN32_NULL, (SIZE_T)size,
                        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

i32 mem_free(void *ptr, usize size)
{
    (void)size;  /* MEM_RELEASE requires dwSize == 0 */
    return VirtualFree((LPVOID)ptr, 0, MEM_RELEASE) ? 0 : -1;
}
