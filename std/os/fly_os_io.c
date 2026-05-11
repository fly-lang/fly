/*===-- std/os/fly_os_io.c - fly.os io implementation -------------------===*/

#include "fly_os_io.h"
#include "fly_os_linux.h"

extern void *malloc (unsigned long);
extern void *realloc(void *, unsigned long);
extern void  free   (void *);

/* ── Internal helpers ────────────────────────────────────────────────────── */

static void cstr_copy(char *d, const char *s, int n) {
    for (int i = 0; i < n; i++) d[i] = s[i];
}

static void fstr_from_buf(const uint8_t *data, int len, fly_string *out) {
    if (len <= 0) { out->ptr = (char *)0; out->size = 0; return; }
    char *p = (char *)malloc((unsigned long)len);
    cstr_copy(p, (const char *)data, len);
    out->ptr  = p;
    out->size = len;
}

static void buf_grow(fly_buf *b, size_t needed) {
    if (b->cap >= needed) return;
    size_t nc = b->cap ? b->cap * 2 : 4096;
    if (nc < needed) nc = needed;
    uint8_t *p = (uint8_t *)realloc(b->ptr, (unsigned long)nc);
    if (!p) return;
    b->ptr = p;
    b->cap = nc;
}

static void buf_append(fly_buf *b, const uint8_t *data, size_t n) {
    buf_grow(b, b->size + n);
    if (!b->ptr) return;
    for (size_t i = 0; i < n; i++) b->ptr[b->size + i] = data[i];
    b->size += n;
}

/* ── fly_string_array builder ────────────────────────────────────────────── */

typedef struct { fly_string *items; int count; int cap; } sa_builder;

static void sa_push(sa_builder *b, fly_string s) {
    if (b->count >= b->cap) {
        int nc = b->cap ? b->cap * 2 : 8;
        fly_string *p = (fly_string *)realloc(b->items,
                            (unsigned long)nc * sizeof(fly_string));
        if (!p) return;
        b->items = p; b->cap = nc;
    }
    b->items[b->count++] = s;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Raw reader                                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_io_read(fly_reader *r, fly_buf *buf, size_t n, size_t *out_read) {
    buf_grow(buf, buf->size + n);
    *out_read = 0;
    if (!buf->ptr || !r || !r->read) return;
    r->read(r->ctx, buf->ptr + buf->size, n, out_read);
    buf->size += *out_read;
}

void fly_io_readAll(fly_reader *r, fly_buf *out) {
    out->ptr  = (uint8_t *)0;
    out->size = 0;
    out->cap  = 0;
    if (!r || !r->read) return;
    uint8_t tmp[4096];
    while (1) {
        size_t nr = 0;
        r->read(r->ctx, tmp, sizeof(tmp), &nr);
        if (nr == 0) break;
        buf_append(out, tmp, nr);
    }
}

void fly_io_readLine(fly_reader *r, fly_string *out) {
    out->ptr = (char *)0; out->size = 0;
    if (!r || !r->read) return;
    /* read byte by byte until '\n' or EOF — simple but portable */
    fly_buf line = {(uint8_t *)0, 0, 0};
    uint8_t b;
    size_t nr;
    while (1) {
        nr = 0;
        r->read(r->ctx, &b, 1, &nr);
        if (nr == 0) break;
        buf_append(&line, &b, 1);
        if (b == '\n') break;
    }
    if (line.size > 0) fstr_from_buf(line.ptr, (int)line.size, out);
    if (line.ptr) free(line.ptr);
}

void fly_io_readLines(fly_reader *r, fly_string_array *out) {
    out->items = (fly_string *)0; out->count = 0;
    sa_builder b = {(fly_string *)0, 0, 0};
    while (1) {
        fly_string line;
        fly_io_readLine(r, &line);
        if (!line.ptr || line.size == 0) break;
        sa_push(&b, line);
    }
    out->items = b.items;
    out->count = b.count;
}

void fly_io_close(fly_reader *r) {
    if (r && r->close) r->close(r->ctx);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Raw writer                                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_io_write(fly_writer *w, const fly_buf *buf, size_t n, size_t *out_written) {
    *out_written = 0;
    if (!w || !w->write || !buf || !buf->ptr) return;
    size_t to_write = n < buf->size ? n : buf->size;
    w->write(w->ctx, buf->ptr, to_write, out_written);
}

void fly_io_writeAll(fly_writer *w, const fly_buf *buf) {
    if (!w || !w->write || !buf || !buf->ptr || buf->size == 0) return;
    size_t written = 0;
    while (written < buf->size) {
        size_t nw = 0;
        w->write(w->ctx, buf->ptr + written, buf->size - written, &nw);
        if (nw == 0) break;
        written += nw;
    }
}

void fly_io_writeString(fly_writer *w, const fly_string *s) {
    if (!w || !w->write || !s || !s->ptr || s->size == 0) return;
    fly_buf tmp;
    tmp.ptr  = (uint8_t *)s->ptr;
    tmp.size = (size_t)s->size;
    tmp.cap  = (size_t)s->size;
    fly_io_writeAll(w, &tmp);
}

void fly_io_flush(fly_writer *w) {
    if (w && w->flush) w->flush(w->ctx);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Buffered reader                                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_io_readerNew(fly_reader *inner, size_t cap, fly_buf_reader *out) {
    out->inner    = *inner;
    out->buf.ptr  = (uint8_t *)malloc((unsigned long)cap);
    out->buf.size = 0;
    out->buf.cap  = cap;
    out->pos      = 0;
}

void fly_io_fill(fly_buf_reader *r) {
    /* move unconsumed bytes to front */
    size_t remaining = r->buf.size - r->pos;
    for (size_t i = 0; i < remaining; i++)
        r->buf.ptr[i] = r->buf.ptr[r->pos + i];
    r->buf.size = remaining;
    r->pos      = 0;
    /* fill remaining capacity */
    if (r->buf.cap > r->buf.size) {
        size_t nr = 0;
        r->inner.read(r->inner.ctx, r->buf.ptr + r->buf.size,
                      r->buf.cap - r->buf.size, &nr);
        r->buf.size += nr;
    }
}

void fly_io_peek(fly_buf_reader *r, size_t n, fly_buf *out) {
    if (r->buf.size - r->pos < n) fly_io_fill(r);
    size_t avail = r->buf.size - r->pos;
    size_t peek  = n < avail ? n : avail;
    out->ptr  = (uint8_t *)malloc((unsigned long)peek);
    out->size = peek;
    out->cap  = peek;
    for (size_t i = 0; i < peek; i++) out->ptr[i] = r->buf.ptr[r->pos + i];
}

void fly_io_bufReadLine(fly_buf_reader *r, fly_string *out) {
    fly_buf line = {(uint8_t *)0, 0, 0};
    while (1) {
        /* search for '\n' in current buffer */
        for (size_t i = r->pos; i < r->buf.size; i++) {
            uint8_t c = r->buf.ptr[i];
            uint8_t tmp[1] = { c };
            buf_append(&line, tmp, 1);
            r->pos = i + 1;
            if (c == '\n') goto done;
        }
        /* need more data */
        size_t old = r->buf.size;
        fly_io_fill(r);
        if (r->buf.size == old) break; /* EOF */
    }
done:
    if (line.size > 0) fstr_from_buf(line.ptr, (int)line.size, out);
    else { out->ptr = (char *)0; out->size = 0; }
    if (line.ptr) free(line.ptr);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Buffered writer                                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_io_writerNew(fly_writer *inner, size_t cap, fly_buf_writer *out) {
    out->inner    = *inner;
    out->buf.ptr  = (uint8_t *)malloc((unsigned long)cap);
    out->buf.size = 0;
    out->buf.cap  = cap;
}

void fly_io_bufFlush(fly_buf_writer *w) {
    if (w->buf.size == 0) return;
    fly_io_writeAll(&w->inner, &w->buf);
    w->buf.size = 0;
}

void fly_io_bufWrite(fly_buf_writer *w, const fly_buf *buf) {
    if (!buf || !buf->ptr || buf->size == 0) return;
    size_t written = 0;
    while (written < buf->size) {
        size_t space = w->buf.cap - w->buf.size;
        size_t chunk = buf->size - written;
        if (chunk > space) chunk = space;
        for (size_t i = 0; i < chunk; i++)
            w->buf.ptr[w->buf.size + i] = buf->ptr[written + i];
        w->buf.size += chunk;
        written     += chunk;
        if (w->buf.size >= w->buf.cap) fly_io_bufFlush(w);
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Stream utilities                                                           */
/* ══════════════════════════════════════════════════════════════════════════ */

void fly_io_copy(fly_reader *src, fly_writer *dst, int64_t *out_copied) {
    *out_copied = 0;
    uint8_t tmp[4096];
    while (1) {
        size_t nr = 0;
        src->read(src->ctx, tmp, sizeof(tmp), &nr);
        if (nr == 0) break;
        size_t written = 0;
        while (written < nr) {
            size_t nw = 0;
            dst->write(dst->ctx, tmp + written, nr - written, &nw);
            if (nw == 0) break;
            written += nw;
        }
        *out_copied += (int64_t)written;
    }
}

void fly_io_copyN(fly_reader *src, fly_writer *dst, size_t n, int64_t *out_copied) {
    *out_copied = 0;
    uint8_t tmp[4096];
    size_t remaining = n;
    while (remaining > 0) {
        size_t chunk = remaining < sizeof(tmp) ? remaining : sizeof(tmp);
        size_t nr = 0;
        src->read(src->ctx, tmp, chunk, &nr);
        if (nr == 0) break;
        size_t written = 0;
        while (written < nr) {
            size_t nw = 0;
            dst->write(dst->ctx, tmp + written, nr - written, &nw);
            if (nw == 0) break;
            written += nw;
        }
        *out_copied += (int64_t)written;
        remaining   -= written;
    }
}

/* ── pipe implementation ─────────────────────────────────────────────────── */

typedef struct { int fds[2]; } pipe_ctx;

static void pipe_read_fn(void *ctx, uint8_t *buf, size_t n, size_t *out_read) {
    pipe_ctx *p = (pipe_ctx *)ctx;
    long r = __os_sc3(SYS_read, (long)p->fds[0], (long)buf, (long)n);
    *out_read = (r > 0) ? (size_t)r : 0;
}
static void pipe_close_r(void *ctx) {
    pipe_ctx *p = (pipe_ctx *)ctx;
    __os_sc1(SYS_close, (long)p->fds[0]);
    free(p);
}

static void pipe_write_fn(void *ctx, const uint8_t *buf, size_t n, size_t *out_written) {
    pipe_ctx *p = (pipe_ctx *)ctx;
    long r = __os_sc3(SYS_write, (long)p->fds[1], (long)buf, (long)n);
    *out_written = (r > 0) ? (size_t)r : 0;
}
static void pipe_flush_fn (void *ctx) { (void)ctx; }
static void pipe_close_w (void *ctx) {
    pipe_ctx *p = (pipe_ctx *)ctx;
    __os_sc1(SYS_close, (long)p->fds[1]);
    /* ctx shared with reader — don't free here */
}

void fly_io_pipe(fly_reader *pipe_r, fly_writer *pipe_w) {
    pipe_ctx *ctx = (pipe_ctx *)malloc(sizeof(pipe_ctx));
    int flags = O_CLOEXEC;
    __os_sc2(SYS_pipe2, (long)ctx->fds, (long)flags);

    pipe_r->ctx   = ctx;
    pipe_r->read  = pipe_read_fn;
    pipe_r->close = pipe_close_r;

    pipe_w->ctx   = ctx;
    pipe_w->write = pipe_write_fn;
    pipe_w->flush = pipe_flush_fn;
    pipe_w->close = pipe_close_w;
}
