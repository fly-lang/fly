/*===-- std/src/fly_os.fly.h - fly.os public Fly header -----------------===
 *
 * Unified public header for the fly.os namespace.
 * Seen by the Fly compiler — documents every exported function with
 * calling convention, parameters, behaviour, and example.
 *
 * Submodules: io, fs, path, env, time
 * C implementation: std/os/fly_os_{io,fs,path,env,time}.{h,c}
 *===----------------------------------------------------------------------===*/

namespace fly.os

/* ══════════════════════════════════════════════════════════════════════════ */
/* Submodule: fly.os.io                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

// read — reads up to n bytes from reader into buf
//
// Invokes the reader's read callback; buf grows automatically.
// out_read receives the number of bytes actually read (0 = EOF).
//
// r        : source reader (file, pipe, memory, …)
// buf      : output buffer; may be uninitialized (ptr=null)
// n        : maximum bytes to read in this call
// out_read : receives the actual byte count read
//
// Example:
//   fly_buf b
//   os.io.read(r, b, 4096, nread)
public read(r Reader, buf Buf, n ulong, out_read ulong)

// readAll — reads the entire reader until EOF
//
// Allocates buf on the heap; caller should free (or let scope do it).
//
// Example:
//   fly_buf content
//   os.io.readAll(r, content)
public readAll(r Reader, out Buf)

// readLine — reads one line (up to and including '\n') from reader
//
// Returns an empty string on EOF.
//
// Example:
//   string line
//   os.io.readLine(r, line)
public readLine(r Reader, out string)

// readLines — reads all lines until EOF, one string per element
//
// Example:
//   string[] lines
//   os.io.readLines(r, lines)
public readLines(r Reader, out string[])

// write — writes up to n bytes from buf to writer
//
// out_written receives the actual byte count written.
//
// Example:
//   ulong written
//   os.io.write(w, buf, buf.size, written)
public write(w Writer, buf Buf, n ulong, out_written ulong)

// writeAll — writes the entire buf, retrying on partial writes
//
// Example:
//   os.io.writeAll(w, data)
public writeAll(w Writer, buf Buf)

// writeString — writes a string to the writer
//
// Example:
//   os.io.writeString(w, "hello\n")
public writeString(w Writer, s string)

// flush — forces pending bytes in internal writer buffer to the underlying sink
//
// No-op for writers with no internal buffer.
//
// Example:
//   os.io.flush(w)
public flush(w Writer)

// readerNew — creates a buffered reader wrapping an existing reader
//
// cap: internal buffer size in bytes (e.g. 4096)
//
// Example:
//   BufReader br
//   os.io.readerNew(r, 4096, br)
public readerNew(inner Reader, cap ulong, out BufReader)

// peek — reads n bytes without consuming them from the buffered reader
//
// Example:
//   fly_buf ahead
//   os.io.peek(br, 4, ahead)
public peek(r BufReader, n ulong, out Buf)

// bufReadLine — reads one line from a buffered reader
//
// More efficient than readLine because it searches the in-memory buffer
// before triggering a syscall.
//
// Example:
//   string line
//   os.io.bufReadLine(br, line)
public bufReadLine(r BufReader, out string)

// fill — refills the internal buffer of a buffered reader from the source
//
// Called automatically when the buffer is empty; rarely needed directly.
public fill(r BufReader)

// writerNew — creates a buffered writer wrapping an existing writer
//
// cap: internal buffer size in bytes (e.g. 4096)
//
// Example:
//   BufWriter bw
//   os.io.writerNew(w, 4096, bw)
public writerNew(inner Writer, cap ulong, out BufWriter)

// bufWrite — writes buf into the buffered writer; flushes automatically if full
//
// Example:
//   os.io.bufWrite(bw, data)
public bufWrite(w BufWriter, buf Buf)

// bufFlush — flushes the buffered writer's internal buffer to the underlying writer
//
// Must be called before closing the writer to avoid losing buffered data.
//
// Example:
//   os.io.bufFlush(bw)
public bufFlush(w BufWriter)

// copy — copies all bytes from src to dst until src returns EOF
//
// out_copied receives the total bytes transferred.
//
// Example:
//   long copied
//   os.io.copy(src, dst, copied)
public copy(src Reader, dst Writer, out_copied long)

// copyN — copies at most n bytes from src to dst
//
// Example:
//   long copied
//   os.io.copyN(src, dst, 1024, copied)
public copyN(src Reader, dst Writer, n ulong, out_copied long)

// pipe — creates a connected reader/writer pair in memory
//
// Bytes written to pipe_w can be read from pipe_r.
// Backed by the kernel pipe(2) syscall.
//
// Example:
//   Reader pr
//   Writer pw
//   os.io.pipe(pr, pw)
//   os.io.writeAll(pw, data)
//   os.io.readAll(pr, result)
public pipe(pipe_r Reader, pipe_w Writer)

/* ══════════════════════════════════════════════════════════════════════════ */
/* Submodule: fly.os.fs                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

// open — opens a file in read-only mode
//
// Fails silently if the file does not exist (out.fd == -1).
// Syscall: open(O_RDONLY)
//
// Example:
//   File f
//   os.fs.open("/etc/hosts", f)
public open(path string, out File)

// create — creates or truncates a file for writing
//
// Syscall: open(O_WRONLY|O_CREAT|O_TRUNC, 0644)
//
// Example:
//   File f
//   os.fs.create("/tmp/out.txt", f)
public create(path string, out File)

// openOpts — opens a file with explicit flags and permissions
//
// flags: FLY_FILE_READ | FLY_FILE_WRITE | FLY_FILE_APPEND | FLY_FILE_CREATE |
//        FLY_FILE_TRUNC | FLY_FILE_EXCL
// perm : Unix permission bits, e.g. 0644 (ignored if file already exists)
//
// Example:
//   File f
//   os.fs.openOpts("/tmp/log", FILE_WRITE|FILE_CREATE|FILE_APPEND, 0644, f)
public openOpts(path string, flags byte, perm uint, out File)

// close — closes an open file descriptor
//
// Sets f.fd = -1 after closing.
//
// Example:
//   os.fs.close(f)
public close(f File)

// reader — wraps a fly_file as a fly_reader
//
// Example:
//   Reader r
//   os.fs.reader(f, r)
public reader(f File, out Reader)

// writer — wraps a fly_file as a fly_writer
//
// Example:
//   Writer w
//   os.fs.writer(f, w)
public writer(f File, out Writer)

// read — reads the entire content of a file into buf
//
// Allocates buf on the heap; Fly frees automatically at end of scope.
// out.size == 0 if the file cannot be read.
// Syscall: open(O_RDONLY) + read() in a loop + close()
//
// Example:
//   Buf content
//   os.fs.read("/etc/hosts", content)
public read(path string, out Buf)

// write — writes data to a file (creates or overwrites)
//
// perm: Unix permission bits for new files (e.g. 0644)
// Syscall: open(O_WRONLY|O_CREAT|O_TRUNC) + write() + close()
//
// Example:
//   os.fs.write("/tmp/out.txt", data, 0644)
public write(path string, data Buf, perm uint)

// append — appends data to a file (creates if missing)
//
// Syscall: open(O_WRONLY|O_CREAT|O_APPEND) + write() + close()
//
// Example:
//   os.fs.append("/tmp/log.txt", entry, 0644)
public append(path string, data Buf, perm uint)

// seekTo — moves the file position to offset relative to whence
//
// whence: SEEK_SET=0 | SEEK_CUR=1 | SEEK_END=2
// out_pos: new absolute position, or -1 on error
// Syscall: lseek(2)
//
// Example:
//   long pos
//   os.fs.seekTo(f, 0, SEEK_END, pos)   // seek to end
public seekTo(f File, offset long, whence int, out_pos long)

// seekPos — returns the current file position
//
// Equivalent to seekTo(f, 0, SEEK_CUR, out_pos)
//
// Example:
//   long pos
//   os.fs.seekPos(f, pos)
public seekPos(f File, out_pos long)

// stat — returns metadata of a file or directory (follows symlinks)
//
// Syscall: stat(2)
// On error, all fields are zero.
//
// Example:
//   Stat s
//   os.fs.stat("/etc/hosts", s)
public stat(path string, out Stat)

// lstat — returns metadata without following symlinks
//
// Syscall: lstat(2)
//
// Example:
//   Stat s
//   os.fs.lstat("/tmp/mylink", s)
public lstat(path string, out Stat)

// size — returns the size in bytes of a file
//
// Returns 0 if the file does not exist or is not readable.
//
// Example:
//   ulong sz
//   os.fs.size("/etc/hosts", sz)
public size(path string, out ulong)

// exists — checks whether a path exists (file or directory)
//
// Example:
//   bool ok
//   os.fs.exists("/tmp", ok)
public exists(path string, out bool)

// sync — flushes kernel file buffers to storage (fsync)
//
// Syscall: fsync(2)
//
// Example:
//   os.fs.sync(f)
public sync(f File)

// truncate — truncates a file to the specified size
//
// Syscall: truncate(2)
//
// Example:
//   os.fs.truncate("/tmp/large.bin", 1024)
public truncate(path string, size ulong)

// chmod — changes file permissions
//
// Syscall: chmod(2)
//
// Example:
//   os.fs.chmod("/tmp/script.sh", 0755)
public chmod(path string, mode uint)

// delete — deletes a file
//
// Syscall: unlink(2)
// Silently fails if the file does not exist.
//
// Example:
//   os.fs.delete("/tmp/tmp.txt")
public delete(path string)

// copy — copies src to dst (creates dst; overwrites if exists)
//
// Example:
//   os.fs.copy("/etc/hosts", "/tmp/hosts.bak")
public copy(src string, dst string)

// move — moves or renames src to dst
//
// Syscall: rename(2)
//
// Example:
//   os.fs.move("/tmp/a.txt", "/tmp/b.txt")
public move(src string, dst string)

// rename — alias for move
public rename(src string, dst string)

// dirCreate — creates a directory
//
// Fails if the directory already exists.
// Syscall: mkdir(2)
//
// Example:
//   os.fs.dirCreate("/tmp/mydir", 0755)
public dirCreate(path string, perm uint)

// dirCreateAll — creates the full directory path (like mkdir -p)
//
// Creates all intermediate directories as needed.
//
// Example:
//   os.fs.dirCreateAll("/tmp/a/b/c", 0755)
public dirCreateAll(path string, perm uint)

// dirDelete — removes an empty directory
//
// Syscall: rmdir(2)
//
// Example:
//   os.fs.dirDelete("/tmp/empty")
public dirDelete(path string)

// dirDeleteAll — removes a directory and all its contents recursively (rm -rf)
//
// Example:
//   os.fs.dirDeleteAll("/tmp/tree")
public dirDeleteAll(path string)

// dirRead — lists the direct contents of a directory
//
// Returns one DirEntry per item; does not recurse.
// Entries "." and ".." are excluded.
// Syscall: getdents64(2)
//
// Example:
//   DirEntry[] entries
//   os.fs.dirRead("/tmp", entries)
public dirRead(path string, out DirEntry[])

// dirWalk — recursively visits every file and directory under path
//
// The callback receives the full path and stat for each entry.
//
// Example:
//   os.fs.dirWalk("/src", (path, stat, data) => { ... })
public dirWalk(path string, callback func, userdata any)

// symlinkCreate — creates a symbolic link: link → target
//
// Syscall: symlink(2)
//
// Example:
//   os.fs.symlinkCreate("/etc/hosts", "/tmp/hosts.lnk")
public symlinkCreate(target string, link string)

// symlinkRead — reads the target of a symbolic link
//
// Returns an empty string if path is not a symlink.
// Syscall: readlink(2)
//
// Example:
//   string dest
//   os.fs.symlinkRead("/tmp/hosts.lnk", dest)
public symlinkRead(path string, out string)

// tempFile — creates a unique temporary file in dir with prefix pattern
//
// Returns the full path and an open file descriptor.
// The caller is responsible for closing and deleting the file.
//
// Example:
//   string tmp_path
//   File   tmp_file
//   os.fs.tempFile("/tmp", "fly_", tmp_path, tmp_file)
public tempFile(dir string, pattern string, out_path string, out_file File)

// tempDir — creates a unique temporary directory in dir with prefix pattern
//
// Example:
//   string tmp_dir
//   os.fs.tempDir("/tmp", "fly_build_", tmp_dir)
public tempDir(dir string, pattern string, out string)

/* ══════════════════════════════════════════════════════════════════════════ */
/* Submodule: fly.os.path                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

// join — joins two path components with the platform separator
//
// Removes duplicate slashes at the join point.
//
// Example:
//   string full
//   os.path.join("/home/user", "docs/file.txt", full)   // "/home/user/docs/file.txt"
public join(base string, component string, out string)

// joinN — joins n path components in sequence
//
// Example:
//   string full
//   string[] parts = ["/a", "b", "c"]
//   os.path.joinN(parts, full)
public joinN(parts string[], out string)

// absolute — returns the canonical absolute path
//
// Prepends cwd if path is relative, then normalizes . and ..
// Syscall: getcwd(2) if path is relative
//
// Example:
//   string abs
//   os.path.absolute("../sibling", abs)
public absolute(path string, out string)

// basename — returns the last component of the path
//
// Strips trailing slashes before extracting the name.
//
// Example:
//   string name
//   os.path.basename("/a/b/file.txt", name)   // "file.txt"
public basename(path string, out string)

// dirname — returns the directory containing the path
//
// Example:
//   string dir
//   os.path.dirname("/a/b/file.txt", dir)     // "/a/b"
public dirname(path string, out string)

// ext — returns the file extension including the dot
//
// Returns "" if there is no extension.
//
// Example:
//   string e
//   os.path.ext("archive.tar.gz", e)          // ".gz"
public ext(path string, out string)

// stem — returns the file name without the last extension
//
// Example:
//   string s
//   os.path.stem("report.pdf", s)             // "report"
public stem(path string, out string)

// split — splits path into (dirname, basename) in one call
//
// Example:
//   string dir, base
//   os.path.split("/a/b/c.txt", dir, base)    // dir="/a/b"  base="c.txt"
public split(path string, out_dir string, out_base string)

// splitExt — splits path into (stem, extension) in one call
//
// Example:
//   string s, e
//   os.path.splitExt("main.go", s, e)         // s="main"  e=".go"
public splitExt(path string, out_stem string, out_ext string)

// isAbsolute — returns true if path starts with the platform separator
//
// Example:
//   bool abs
//   os.path.isAbsolute("/etc/hosts", abs)     // true
public isAbsolute(path string, out bool)

// isRelative — returns true if path does not start with the separator
public isRelative(path string, out bool)

// normalize — collapses redundant separators and resolves . and ..
//
// "/a/b/../c/./d" → "/a/c/d"
//
// Example:
//   string n
//   os.path.normalize("/a/b/../c", n)
public normalize(path string, out string)

// normalizeInPlace — normalizes path in place
//
// Example:
//   os.path.normalizeInPlace(path)
public normalizeInPlace(path string)

// rel — computes the relative path from base to target
//
// base="/a/b"  target="/a/c/d"  → "../c/d"
//
// Example:
//   string r
//   os.path.rel("/a/b", "/a/c/d", r)
public rel(base string, target string, out string)

// isFile — returns true if path refers to a regular file (follows symlinks)
//
// Syscall: lstat(2)
//
// Example:
//   bool ok
//   os.path.isFile("/etc/hosts", ok)
public isFile(path string, out bool)

// isDir — returns true if path refers to a directory (follows symlinks)
public isDir(path string, out bool)

// isSym — returns true if path is a symbolic link
public isSym(path string, out bool)

// glob — expands a glob pattern and returns all matching paths
//
// Supported wildcards: * (any sequence), ? (any char), [abc] (char class)
// Only the filename part of the pattern is matched; the directory is listed.
//
// Example:
//   string[] matches
//   os.path.glob("/src/**/*.fly", matches)
public glob(pattern string, out string[])

// match — tests whether name matches the glob pattern
//
// Pattern syntax: * ? [abc] [a-z] [!abc]
//
// Example:
//   bool ok
//   os.path.match("*.fly", "main.fly", ok)    // true
public match(pattern string, name string, out bool)

// comp — splits path into its individual components
//
// "/a/b/c" → ["/", "a", "b", "c"]
// "x/y"   → ["x", "y"]
//
// Example:
//   string[] parts
//   os.path.comp("/usr/local/bin", parts)
public comp(path string, out string[])

// sep — returns the path separator character for the current platform
//
// '/' on Linux/macOS, '\' on Windows
public sep(out byte)

/* ══════════════════════════════════════════════════════════════════════════ */
/* Submodule: fly.os.env                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

// get — reads a single environment variable by key
//
// Returns an empty string if the variable is not set.
//
// Example:
//   string home
//   os.env.get("HOME", home)
public get(key string, out string)

// set — sets an environment variable (overwrites if exists)
//
// Example:
//   os.env.set("MY_VAR", "value")
public set(key string, value string)

// delete — removes an environment variable
//
// Silently does nothing if the variable is not set.
//
// Example:
//   os.env.delete("TMP_VAR")
public delete(key string)

// all — returns all environment variables as "KEY=VALUE" strings
//
// Example:
//   string[] env
//   os.env.all(env)
public all(out string[])

// expand — expands $VAR and ${VAR} references in a string
//
// Uses the current process environment for lookups.
//
// Example:
//   string result
//   os.env.expand("$HOME/docs", result)       // "/home/user/docs"
public expand(s string, out string)

// cwdGet — returns the current working directory
//
// Syscall: getcwd(2)
//
// Example:
//   string cwd
//   os.env.cwdGet(cwd)
public cwdGet(out string)

// cwdSet — changes the current working directory
//
// Syscall: chdir(2)
//
// Example:
//   os.env.cwdSet("/tmp")
public cwdSet(path string)

// argsGet — returns the command-line arguments (argv[0] is the program name)
//
// Requires fly_env_init() to be called from the program entry point.
//
// Example:
//   string[] args
//   os.env.argsGet(args)
public argsGet(out string[])

// argsCount — returns the number of command-line arguments (including argv[0])
//
// Example:
//   int n
//   os.env.argsCount(n)
public argsCount(out int)

// hostname — returns the machine hostname
//
// Syscall: uname(2), nodename field
//
// Example:
//   string host
//   os.env.hostname(host)
public hostname(out string)

// osname — returns the operating system name as a lowercase string
//
// "linux" | "macos" | "windows"
//
// Example:
//   string os_name
//   os.env.osname(os_name)
public osname(out string)

// exit — terminates the process with the given exit code
//
// This function never returns.
// Syscall: exit_group(2)
//
// Example:
//   os.env.exit(0)
public exit(code int)

/* ══════════════════════════════════════════════════════════════════════════ */
/* Submodule: fly.os.time                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

// now — returns the current wall-clock time
//
// Syscall: clock_gettime(CLOCK_REALTIME)
//
// Example:
//   Time t
//   os.time.now(t)
public now(out Time)

// monotonic — returns a monotonically increasing timestamp
//
// Suitable for measuring elapsed time, not for wall-clock readings.
// Syscall: clock_gettime(CLOCK_MONOTONIC)
//
// Example:
//   Time start
//   os.time.monotonic(start)
public monotonic(out Time)

// sleep — suspends the current thread for the specified duration
//
// d.nsec uses FLY_MILLISECOND / FLY_SECOND etc. constants.
// Syscall: nanosleep(2)
//
// Example:
//   Duration d
//   d.nsec = 500 * FLY_MILLISECOND
//   os.time.sleep(d)
public sleep(d Duration)

// since — returns the duration elapsed since timestamp t
//
// Equivalent to diff(t, now()).
//
// Example:
//   Duration elapsed
//   os.time.since(start, elapsed)
public since(t Time, out Duration)

// diff — returns b - a as a Duration
//
// A negative result means a is after b.
//
// Example:
//   Duration d
//   os.time.diff(a, b, d)
public diff(a Time, b Time, out Duration)

// add — adds a Duration to a Time, returning a new Time
//
// Example:
//   Time future
//   os.time.add(now, d, future)
public add(t Time, d Duration, out Time)

// compare — compares two timestamps: -1 if a<b, 0 if equal, 1 if a>b
//
// Example:
//   int cmp
//   os.time.compare(a, b, cmp)
public compare(a Time, b Time, out int)

// unix — returns the Unix timestamp as integer seconds
//
// Example:
//   long sec
//   os.time.unix(t, sec)
public unix(t Time, out long)

// unixNano — returns the Unix timestamp in nanoseconds
//
// Example:
//   long nsec
//   os.time.unixNano(t, nsec)
public unixNano(t Time, out long)

// fromUnix — constructs a Time from Unix seconds
//
// Example:
//   Time t
//   os.time.fromUnix(1700000000, t)
public fromUnix(sec long, out Time)

// fromUnixNano — constructs a Time from Unix nanoseconds
//
// Example:
//   Time t
//   os.time.fromUnixNano(1700000000000000000, t)
public fromUnixNano(nsec long, out Time)

// format — formats a timestamp using a Go-style reference pattern
//
// Reference tokens: 2006 01 02 15 04 05 January Jan Monday Mon
// Non-token characters are copied verbatim.
//
// Example:
//   string s
//   os.time.format(t, "2006-01-02T15:04:05", s)
public format(t Time, pattern string, out string)

// parse — parses a string into a Time using a Go-style reference pattern
//
// Supports the same tokens as format.
// On parse error, out is the zero Time.
//
// Example:
//   Time t
//   os.time.parse("2024-03-07", "2006-01-02", t)
public parse(s string, pattern string, out Time)

// durationSecs — converts a Duration to whole seconds (truncated)
//
// Example:
//   long s
//   os.time.durationSecs(d, s)
public durationSecs(d Duration, out long)

// durationMillis — converts a Duration to whole milliseconds (truncated)
public durationMillis(d Duration, out long)

// durationMicros — converts a Duration to whole microseconds (truncated)
public durationMicros(d Duration, out long)

// durationFormat — returns a human-readable representation of a Duration
//
// Examples: "0s", "200ms", "3.500s", "1h30m00s", "42ns"
//
// Example:
//   string s
//   os.time.durationFormat(d, s)
public durationFormat(d Duration, out string)
