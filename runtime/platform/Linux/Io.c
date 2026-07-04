/*===-- runtime/Linux/Io.c - Basic I/O via libc write ----------------===*/

#include "../Runtime.h"

/* libc (forward-declared; the runtime builds with -nostdinc, like Mem.c). */
extern i64 write(i32 fd, const void *buf, usize count);

i64 io_write(i32 fd, const void *buf, usize count)
{
    i64 r = write(fd, buf, count);
    return (r >= 0) ? r : -1;
}
