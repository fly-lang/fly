/*===-- runtime/Windows/Proc.c - Process control via ExitProcess ------===*/

#include "../Runtime.h"
#include "Win32.h"

FLY_NORETURN void proc_exit(i32 code)
{
    ExitProcess((DWORD)(unsigned int)code);
    FLY_UNREACHABLE();
}
