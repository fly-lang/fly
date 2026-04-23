/*===-- src/Runtime/Linux/Proc.c - Process control via exit_group ---------===*/

#include "Runtime/Runtime.h"
#include "Syscall.h"

__attribute__((noreturn)) void proc_exit(i32 code)
{
    __syscall1(SYS_exit_group, (long)code);
    __builtin_unreachable();
}
