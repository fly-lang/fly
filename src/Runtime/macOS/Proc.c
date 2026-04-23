/*===-- src/Runtime/macOS/Proc.c - Process control via _exit --------------===*/

#include "Runtime/Runtime.h"
#include "LibSystem.h"

__attribute__((noreturn)) void proc_exit(i32 code)
{
    _exit(code);
}
