/*===-- src/Runtime/Windows/Proc.c - Process control via ExitProcess ------===*/

#include "Runtime/Runtime.h"
#include "Win32.h"

__attribute__((noreturn)) void proc_exit(i32 code)
{
    ExitProcess((DWORD)(unsigned int)code);
    __builtin_unreachable();
}
