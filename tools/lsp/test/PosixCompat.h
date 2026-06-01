//===----------------------------------------------------------------------===//
// tools/lsp/test/PosixCompat.h — cross-platform POSIX I/O for LSP tests
//
// On Windows/MSVC:
//   • <unistd.h> does not exist; the equivalents live in <io.h>.
//   • POSIX function names (dup, close, read, write, pipe) use an underscore
//     prefix: _dup, _close, _read, _write, _pipe.
//   • STDIN_FILENO / STDOUT_FILENO must be defined manually.
//   • ssize_t is undefined — map to int.
//   • Non-blocking pipe reads use PeekNamedPipe instead of O_NONBLOCK.
//===----------------------------------------------------------------------===//
#pragma once

#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#  include <windows.h>
#  include <algorithm>
#  ifndef STDIN_FILENO
#    define STDIN_FILENO  0
#  endif
#  ifndef STDOUT_FILENO
#    define STDOUT_FILENO 1
#  endif
// Map POSIX names to their MSVC underscore equivalents.
// Use inline wrappers to avoid macro-expansion surprises with ::write etc.
inline int  posix_pipe (int fds[2]) { return _pipe(fds, 65536, _O_BINARY); }
inline int  posix_dup  (int fd)               { return _dup(fd); }
inline int  posix_dup2 (int src, int dst)     { return _dup2(src, dst); }
inline int  posix_close(int fd)               { return _close(fd); }
inline int  posix_read (int fd, void *buf, unsigned n) { return _read(fd, buf, n); }
inline int  posix_write(int fd, const void *buf, unsigned n) { return _write(fd, buf, n); }
#  define pipe(fds)          posix_pipe(fds)
#  define dup(fd)            posix_dup(fd)
#  define dup2(src,dst)      posix_dup2(src,dst)
#  define close(fd)          posix_close(fd)
#  define read(fd,buf,n)     posix_read(fd, buf, (unsigned)(n))
#  define write(fd,buf,n)    posix_write(fd, buf, (unsigned)(n))
// ssize_t: do not redefine here — LLVM/SDK headers may already define it as
// __int64. Callers should use `int` for read/write return values on Windows.
#else
#  include <unistd.h>
#  include <fcntl.h>
#endif

#include <string>

// Drain all bytes currently available in the read end of a pipe.
// After fflush(stdout) the writer has flushed; this reads without blocking.
//   POSIX   : temporarily enables O_NONBLOCK.
//   Windows : uses PeekNamedPipe to avoid a blocking read.
inline std::string drainPipe(int fd) {
    std::string result;
    char buf[4096];
#ifdef _WIN32
    HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    DWORD avail = 0;
    while (PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL) && avail > 0) {
        DWORD toRead = (std::min)(static_cast<DWORD>(sizeof(buf)), avail);
        int n = _read(fd, buf, static_cast<unsigned>(toRead));
        if (n > 0) result.append(buf, static_cast<size_t>(n));
    }
#else
    int fl = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    ssize_t n;
    while ((n = ::read(fd, buf, sizeof(buf))) > 0)
        result.append(buf, static_cast<size_t>(n));
    fcntl(fd, F_SETFL, fl);
#endif
    return result;
}
