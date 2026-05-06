/*===-- runtime/macOS/Proc.c - Process control via _exit --------------===*/

#include "../Runtime.h"
#include "LibSystem.h"

FLY_NORETURN void proc_exit(i32 code)
{
    _exit(code);
    FLY_UNREACHABLE();
}
