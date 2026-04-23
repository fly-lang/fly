/*===-- src/Runtime/macOS/Io.c - Basic I/O via write ----------------------===*/

#include "Runtime/Runtime.h"
#include "LibSystem.h"

i64 io_write(i32 fd, const void *buf, usize count)
{
    return (i64)write(fd, buf, (size_rt)count);
}
