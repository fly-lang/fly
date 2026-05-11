/*===-- std/os/fly_os_io.h - fly.os shared types and io API -------------===
 *
 * Public header — included by all fly.os submodule headers and by user code.
 * No libc headers. All types defined via compiler built-ins.
 *
 * Shared types: fly_buf, fly_reader, fly_writer, fly_file, fly_stat,
 *               fly_dir_entry, fly_dir_entries, fly_time, fly_duration.
 * Also re-exports fly_string / fly_string_array from fly_str.h.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_IO_H
#define FLY_OS_IO_H

#include "fly_str.h"   /* fly_string, fly_string_array */

/* ── Portable integer types (no system headers) ──────────────────────────── */

typedef __UINT8_TYPE__   uint8_t;
typedef __UINT16_TYPE__  uint16_t;
typedef __UINT32_TYPE__  uint32_t;
typedef __UINT64_TYPE__  uint64_t;
typedef __INT32_TYPE__   int32_t;
typedef __INT64_TYPE__   int64_t;
typedef __SIZE_TYPE__    size_t;

/* ── File open flags (FLY_FILE_*) ────────────────────────────────────────── */

#define FLY_FILE_READ    0x01
#define FLY_FILE_WRITE   0x02
#define FLY_FILE_APPEND  0x04
#define FLY_FILE_CREATE  0x08
#define FLY_FILE_TRUNC   0x10
#define FLY_FILE_EXCL    0x20

/* ── Seek whence ─────────────────────────────────────────────────────────── */

#define FLY_SEEK_SET  0
#define FLY_SEEK_CUR  1
#define FLY_SEEK_END  2

/* ── Duration constants (nanoseconds) ───────────────────────────────────── */

#define FLY_NANOSECOND   1LL
#define FLY_MICROSECOND  1000LL
#define FLY_MILLISECOND  1000000LL
#define FLY_SECOND       1000000000LL
#define FLY_MINUTE       60000000000LL
#define FLY_HOUR         3600000000000LL

/* ══════════════════════════════════════════════════════════════════════════ */
/* Shared data types                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

/* Generic byte buffer — same growth pattern as fly_string */
typedef struct {
    uint8_t *ptr;   /* heap-allocated; NULL = not yet allocated */
    size_t   size;  /* bytes written                            */
    size_t   cap;   /* bytes allocated                          */
} fly_buf;

/* Reader trait — abstraction over any readable source */
typedef struct fly_reader fly_reader;
struct fly_reader {
    void  *ctx;
    void (*read) (void *ctx, uint8_t *buf, size_t n, size_t *out_read);
    void (*close)(void *ctx);
};

/* Writer trait — abstraction over any writable destination */
typedef struct fly_writer fly_writer;
struct fly_writer {
    void  *ctx;
    void (*write)(void *ctx, const uint8_t *buf, size_t n, size_t *out_written);
    void (*flush)(void *ctx);
    void (*close)(void *ctx);
};

/* Buffered reader — wraps fly_reader with an internal buffer */
typedef struct {
    fly_reader inner;
    fly_buf    buf;
    size_t     pos;   /* current read position within buf */
} fly_buf_reader;

/* Buffered writer — wraps fly_writer with an internal buffer */
typedef struct {
    fly_writer inner;
    fly_buf    buf;
} fly_buf_writer;

/* File descriptor */
typedef struct {
    int     fd;     /* -1 = not open */
    uint8_t flags;  /* FLY_FILE_* bitmask */
} fly_file;

/* File/directory metadata */
typedef struct {
    uint64_t size;
    uint64_t mtime_sec;
    uint64_t mtime_nsec;
    uint32_t mode;
    int      is_file;    /* 1 if regular file */
    int      is_dir;     /* 1 if directory    */
    int      is_symlink; /* 1 if symlink       */
} fly_stat;

/* Single directory entry */
typedef struct {
    fly_string name;
    fly_stat   stat;
} fly_dir_entry;

/* Array of directory entries */
typedef struct {
    fly_dir_entry *items;
    size_t         len;
    size_t         cap;
} fly_dir_entries;

/* Absolute timestamp */
typedef struct {
    int64_t sec;   /* Unix seconds  */
    int64_t nsec;  /* Nanoseconds   */
} fly_time;

/* Duration (signed, in nanoseconds) */
typedef struct {
    int64_t nsec;  /* negative = in the past */
} fly_duration;

/* ══════════════════════════════════════════════════════════════════════════ */
/* fly.io API  (ABI-mangled names, prefix _F6fly_os)                         */
/* ══════════════════════════════════════════════════════════════════════════ */

/* ── Raw reader/writer ───────────────────────────────────────────────────── */

void _F6fly_os6ioRead_Cfly_reader_Cfly_buf_ul_ul    (void *err_ctx, fly_reader *r, fly_buf *buf, size_t n, size_t *out_read);
void _F6fly_os9ioReadAll_Cfly_reader_Cfly_buf      (void *err_ctx, fly_reader *r, fly_buf *out);
void _F6fly_os10ioReadLine_Cfly_reader_Ss           (void *err_ctx, fly_reader *r, fly_string *out);
void _F6fly_os11ioReadLines_Cfly_reader_Cfly_string_array (void *err_ctx, fly_reader *r, fly_string_array *out);
void _F6fly_os7ioClose_Cfly_reader                 (void *err_ctx, fly_reader *r);

void _F6fly_os7ioWrite_Cfly_writer_Cfly_buf_ul_ul   (void *err_ctx, fly_writer *w, const fly_buf *buf, size_t n, size_t *out_written);
void _F6fly_os10ioWriteAll_Cfly_writer_Cfly_buf    (void *err_ctx, fly_writer *w, const fly_buf *buf);
void _F6fly_os13ioWriteString_Cfly_writer_Ss        (void *err_ctx, fly_writer *w, const fly_string *s);
void _F6fly_os7ioFlush_Cfly_writer                 (void *err_ctx, fly_writer *w);

/* ── Buffered reader ─────────────────────────────────────────────────────── */

void _F6fly_os11ioReaderNew_Cfly_reader_ul_Cfly_buf_reader (void *err_ctx, fly_reader *inner, size_t cap, fly_buf_reader *out);
void _F6fly_os6ioPeek_Cfly_buf_reader_ul_Cfly_buf          (void *err_ctx, fly_buf_reader *r, size_t n, fly_buf *out);
void _F6fly_os13ioBufReadLine_Cfly_buf_reader_Ss            (void *err_ctx, fly_buf_reader *r, fly_string *out);
void _F6fly_os6ioFill_Cfly_buf_reader                      (void *err_ctx, fly_buf_reader *r);

/* ── Buffered writer ─────────────────────────────────────────────────────── */

void _F6fly_os11ioWriterNew_Cfly_writer_ul_Cfly_buf_writer (void *err_ctx, fly_writer *inner, size_t cap, fly_buf_writer *out);
void _F6fly_os10ioBufWrite_Cfly_buf_writer_Cfly_buf        (void *err_ctx, fly_buf_writer *w, const fly_buf *buf);
void _F6fly_os10ioBufFlush_Cfly_buf_writer                 (void *err_ctx, fly_buf_writer *w);

/* ── Stream utilities ────────────────────────────────────────────────────── */

void _F6fly_os6ioCopy_Cfly_reader_Cfly_writer_l    (void *err_ctx, fly_reader *src, fly_writer *dst, int64_t *out_copied);
void _F6fly_os7ioCopyN_Cfly_reader_Cfly_writer_ul_l (void *err_ctx, fly_reader *src, fly_writer *dst, size_t n, int64_t *out_copied);
void _F6fly_os6ioPipe_Cfly_reader_Cfly_writer       (void *err_ctx, fly_reader *pipe_r, fly_writer *pipe_w);

#endif /* FLY_OS_IO_H */
