/*===-- runtime/Linux/Proc.c - Process control via exit_group ---------===*/

#include "../Runtime.h"
#include "Syscall.h"

FLY_NORETURN void proc_exit(i32 code)
{
    __syscall1(SYS_exit_group, (long)code);
    FLY_UNREACHABLE();
}
