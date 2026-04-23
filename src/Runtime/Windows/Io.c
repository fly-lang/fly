/*===-- src/Runtime/Windows/Io.c - Basic I/O via WriteFile ----------------===*/

#include "Runtime/Runtime.h"
#include "Win32.h"

i64 io_write(i32 fd, const void *buf, usize count)
{
    DWORD std_handle;
    switch (fd) {
        case STDOUT: std_handle = STD_OUTPUT_HANDLE; break;
        case STDERR: std_handle = STD_ERROR_HANDLE;  break;
        default:     return -1;
    }
    HANDLE h = GetStdHandle(std_handle);
    if (h == INVALID_HANDLE || h == WIN32_NULL)
        return -1;
    if (count > 0xFFFFFFFFULL)
        return -1;
    DWORD written = 0;
    return WriteFile(h, buf, (DWORD)count, &written, WIN32_NULL)
           ? (i64)written : -1;
}
