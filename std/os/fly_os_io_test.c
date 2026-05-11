/*===-- std/os/fly_os_io_test.c - freestanding tests for fly_os_io ------===*/

#include "fly_os_io.h"
#include "fly_os_fs.h"

extern void free(void *);

/* ── Test harness ────────────────────────────────────────────────────────── */

static __attribute__((noinline)) void fly_write_fd(int fd, const char *buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"((long)fd), "S"(buf), "d"(len) : "rcx", "r11", "memory");
}
static void fly_write(const char *buf, long len) { fly_write_fd(1, buf, len); }
static void fly_puts(const char *s){long n=0;while(s[n])n++;fly_write(s,n);}
static void fly_put_u64(unsigned long long v){
    if(!v){fly_write("0",1);return;}
    char buf[20];int len=0;
    while(v){buf[len++]=(char)('0'+(int)(v%10));v/=10;}
    for(int lo=0,hi=len-1;lo<hi;lo++,hi--){char t=buf[lo];buf[lo]=buf[hi];buf[hi]=t;}
    fly_write(buf,(long)len);
}
static __attribute__((noreturn,noinline)) void fly_exit(int code){
    __asm__ volatile("syscall":: "a"(60L),"D"((long)code):"rcx","r11","memory");
    __builtin_unreachable();
}
static int g_pass=0,g_fail=0;
static void check(int cond,const char *name){
    if(cond)g_pass++;
    else{g_fail++;fly_puts("FAIL: ");fly_puts(name);fly_write("\n",1);}
}

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static fly_string fs_s(const char *s){
    fly_string r;int n=0;while(s[n])n++;r.ptr=(char*)s;r.size=n;return r;
}
static void cleanup_b(fly_buf *b){
    if(b->ptr){free(b->ptr);b->ptr=(uint8_t*)0;b->size=0;b->cap=0;}
}

/* In-memory reader over a fixed buffer */
typedef struct { const uint8_t *data; size_t len; size_t pos; } mem_ctx;

static void mem_read(void *ctx, uint8_t *buf, size_t n, size_t *out_read) {
    mem_ctx *m = (mem_ctx *)ctx;
    size_t avail = m->len - m->pos;
    size_t rd = n < avail ? n : avail;
    for (size_t i = 0; i < rd; i++) buf[i] = m->data[m->pos + i];
    m->pos    += rd;
    *out_read  = rd;
}
static void mem_close(void *ctx) { (void)ctx; }

static fly_reader make_mem_reader(mem_ctx *ctx, const char *data, int len) {
    ctx->data = (const uint8_t *)data;
    ctx->len  = (size_t)len;
    ctx->pos  = 0;
    fly_reader r;
    r.ctx   = ctx;
    r.read  = mem_read;
    r.close = mem_close;
    return r;
}

/* In-memory writer into a fly_buf */
static void buf_write(void *ctx, const uint8_t *data, size_t n, size_t *out_written) {
    fly_buf *b = (fly_buf *)ctx;
    size_t needed = b->size + n;
    if (b->cap < needed) {
        size_t nc = b->cap ? b->cap * 2 : 256;
        if (nc < needed) nc = needed;
        uint8_t *p = (uint8_t *)__builtin_alloca(0); /* suppress warning — use realloc */
        (void)p;
        extern void *realloc(void *, unsigned long);
        uint8_t *np = (uint8_t *)realloc(b->ptr, (unsigned long)nc);
        if (!np) { *out_written = 0; return; }
        b->ptr = np; b->cap = nc;
    }
    for (size_t i = 0; i < n; i++) b->ptr[b->size + i] = data[i];
    b->size      += n;
    *out_written  = n;
}
static void buf_flush(void *ctx) { (void)ctx; }
static void buf_close_w(void *ctx) { (void)ctx; }

static fly_writer make_buf_writer(fly_buf *b) {
    fly_writer w;
    w.ctx   = b;
    w.write = buf_write;
    w.flush = buf_flush;
    w.close = buf_close_w;
    return w;
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_pipe(void) {                       /* 1 */
    fly_reader pr; fly_writer pw;
    _F6fly_os6ioPipe_Cfly_reader_Cfly_writer((void*)0, &pr, &pw);

    const char *msg = "hello pipe";
    int mlen = 0; while(msg[mlen])mlen++;
    fly_buf wb; wb.ptr=(uint8_t*)msg; wb.size=(size_t)mlen; wb.cap=(size_t)mlen;
    _F6fly_os10ioWriteAll_Cfly_writer_Cfly_buf((void*)0, &pw, &wb);

    /* close write end before reading so read sees EOF */
    pw.close(pw.ctx);

    fly_buf rb = {(uint8_t*)0,0,0};
    _F6fly_os9ioReadAll_Cfly_reader_Cfly_buf((void*)0, &pr, &rb);

    check(rb.size == (size_t)mlen, "pipe size matches");
    int ok = 1;
    for (int i=0;i<mlen;i++) if((char)rb.ptr[i]!=msg[i]){ok=0;break;}
    check(ok, "pipe content matches");
    cleanup_b(&rb);
    pr.close(pr.ctx);
}

static void test_mem_reader_bufreadline(void) {     /* 2 */
    const char *data = "line one\nline two\nline three\n";
    int dlen = 0; while(data[dlen])dlen++;
    mem_ctx mctx;
    fly_reader r = make_mem_reader(&mctx, data, dlen);
    fly_buf_reader br;
    _F6fly_os11ioReaderNew_Cfly_reader_ul_Cfly_buf_reader((void*)0, &r, 16, &br); /* small buffer to force refill */

    fly_string line1, line2, line3;
    _F6fly_os13ioBufReadLine_Cfly_buf_reader_Ss((void*)0, &br, &line1);
    _F6fly_os13ioBufReadLine_Cfly_buf_reader_Ss((void*)0, &br, &line2);
    _F6fly_os13ioBufReadLine_Cfly_buf_reader_Ss((void*)0, &br, &line3);

    /* lines include '\n' */
    check(line1.size > 0 && line1.ptr[0]=='l', "bufReadLine line1 starts with 'l'");
    check(line2.size > 0, "bufReadLine line2 non-empty");
    check(line3.size > 0, "bufReadLine line3 non-empty");

    extern void free(void *);
    if(line1.ptr) free(line1.ptr);
    if(line2.ptr) free(line2.ptr);
    if(line3.ptr) free(line3.ptr);
    if(br.buf.ptr) free(br.buf.ptr);
}

static void test_buf_writer(void) {                 /* 3 */
    fly_buf dst = {(uint8_t*)0,0,0};
    fly_writer w = make_buf_writer(&dst);
    fly_buf_writer bw;
    _F6fly_os11ioWriterNew_Cfly_writer_ul_Cfly_buf_writer((void*)0, &w, 8, &bw); /* small buffer to force flushes */

    const char *msg = "buffered write test";
    int mlen=0; while(msg[mlen])mlen++;
    fly_buf sb; sb.ptr=(uint8_t*)msg; sb.size=(size_t)mlen; sb.cap=(size_t)mlen;
    _F6fly_os10ioBufWrite_Cfly_buf_writer_Cfly_buf((void*)0, &bw, &sb);
    _F6fly_os10ioBufFlush_Cfly_buf_writer((void*)0, &bw);

    check(dst.size == (size_t)mlen, "bufWriter size");
    int ok=1;
    for(int i=0;i<mlen;i++) if((char)dst.ptr[i]!=msg[i]){ok=0;break;}
    check(ok, "bufWriter content");
    cleanup_b(&dst);
    if(bw.buf.ptr) free(bw.buf.ptr);
}

static void test_copy(void) {                       /* 4 */
    const char *data = "stream copy test data 1234567890";
    int dlen=0; while(data[dlen])dlen++;
    mem_ctx mctx;
    fly_reader r = make_mem_reader(&mctx, data, dlen);
    fly_buf dst = {(uint8_t*)0,0,0};
    fly_writer w = make_buf_writer(&dst);

    int64_t copied;
    _F6fly_os6ioCopy_Cfly_reader_Cfly_writer_l((void*)0, &r, &w, &copied);

    check(copied == (int64_t)dlen, "copy bytes count");
    int ok=1;
    for(int i=0;i<dlen;i++) if((char)dst.ptr[i]!=data[i]){ok=0;break;}
    check(ok, "copy content");
    cleanup_b(&dst);
}

static void run_tests(void);
__attribute__((naked)) void _start(void) {
    __asm__ volatile("andq $-16,%%rsp\ncallq %P0\n"::"i"(run_tests));
}
static void run_tests(void) {
    test_pipe();
    test_mem_reader_bufreadline();
    test_buf_writer();
    test_copy();

    unsigned long total=(unsigned long)(g_pass+g_fail);
    fly_puts("fly_os_io: ");
    fly_put_u64((unsigned long long)g_pass);fly_write("/",1);
    fly_put_u64((unsigned long long)total);fly_puts(" passed");
    if(g_fail){fly_puts("  (");fly_put_u64((unsigned long long)g_fail);fly_puts(" FAILED)");}
    fly_write("\n",1);
    fly_exit(g_fail?1:0);
}
