/*===-- runtime/Linux/Io.c - Basic I/O via write syscall --------------===*/

#include "../Runtime.h"
#include "Syscall.h"

i64 io_write(i32 fd, const void *buf, usize count)
{
    return (i64)__syscall3(SYS_write, (long)fd, (long)buf, (long)count);
}
