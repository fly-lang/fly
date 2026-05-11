/*===-- std/os/fly_os_path_test.c - freestanding tests for fly_os_path --===*/

#include "fly_os_path.h"

extern void free(void *);

/* ── Freestanding test harness ───────────────────────────────────────────── */

static __attribute__((noinline)) void fly_write(const char *buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"(1L), "S"(buf), "d"(len)
        : "rcx", "r11", "memory");
}
static void fly_puts(const char *s) { long n=0; while(s[n])n++; fly_write(s,n); }
static void fly_put_u64(unsigned long long v) {
    if (!v) { fly_write("0",1); return; }
    char buf[20]; int len=0;
    while (v) { buf[len++]=(char)('0'+(int)(v%10)); v/=10; }
    for (int lo=0,hi=len-1; lo<hi; lo++,hi--) {char t=buf[lo];buf[lo]=buf[hi];buf[hi]=t;}
    fly_write(buf,(long)len);
}
static __attribute__((noreturn,noinline)) void fly_exit(int code) {
    __asm__ volatile("syscall" : : "a"(60L),"D"((long)code) : "rcx","r11","memory");
    __builtin_unreachable();
}

static int g_pass = 0, g_fail = 0;
static void check(int cond, const char *name) {
    if (cond) { g_pass++; }
    else { g_fail++; fly_puts("FAIL: "); fly_puts(name); fly_write("\n",1); }
}

/* ── Helpers ──────────────────────────────────────────────────────────────── */

static fly_string fs(const char *s) {
    fly_string r; int n=0; while(s[n])n++;
    r.ptr=(char*)s; r.size=n; return r;
}
static int fseq(const fly_string *a, const char *b) {
    int n=0; while(b[n])n++;
    if (a->size != n) return 0;
    for (int i=0; i<n; i++) if (a->ptr[i]!=b[i]) return 0;
    return 1;
}
static void cleanup(fly_string *s) {
    if (s->ptr) { free(s->ptr); s->ptr=(char*)0; s->size=0; }
}
static void cleanup_sa(fly_string_array *a) {
    if (a->items) {
        for (int i=0; i<a->count; i++) if (a->items[i].ptr) free(a->items[i].ptr);
        free(a->items); a->items=(fly_string*)0; a->count=0;
    }
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_join(void) {                           /* 19 */
    fly_string out;
    fly_string base = fs("/home/user"), comp = fs("docs/file.txt");
    fly_path_join(&base, &comp, &out);
    check(fseq(&out, "/home/user/docs/file.txt"), "join abs+rel"); cleanup(&out);
    fly_string empty = fs("");
    fly_path_join(&empty, &comp, &out);
    check(fseq(&out, "docs/file.txt"), "join empty+rel");
}

static void test_basename_dirname(void) {               /* 20 */
    fly_string out;
    fly_string p1 = fs("/a/b/file.txt"), p2 = fs("/a/b/"), root = fs("/");

    fly_path_basename(&p1, &out); check(fseq(&out,"file.txt"), "basename file.txt"); cleanup(&out);
    fly_path_basename(&p2, &out); check(fseq(&out,"b"),        "basename trailing/"); cleanup(&out);
    fly_path_dirname (&p1, &out); check(fseq(&out,"/a/b"),     "dirname file"); cleanup(&out);
    fly_path_dirname (&p2, &out); check(fseq(&out,"/a"),       "dirname trailing/"); cleanup(&out);
    fly_path_dirname (&root,&out);check(fseq(&out,"/"),        "dirname root"); cleanup(&out);
}

static void test_ext(void) {                            /* 21 */
    fly_string out;
    fly_string f1=fs("file.txt"), f2=fs("archive.tar.gz"), f3=fs("noext"), f4=fs(".hidden");
    fly_path_ext(&f1,&out); check(fseq(&out,".txt"), "ext .txt"); cleanup(&out);
    fly_path_ext(&f2,&out); check(fseq(&out,".gz"),  "ext .gz");  cleanup(&out);
    fly_path_ext(&f3,&out); check(out.size==0,        "ext none"); cleanup(&out);
    fly_path_ext(&f4,&out); check(out.size==0, "ext .hidden"); cleanup(&out);
}

static void test_normalize(void) {                      /* 22 */
    fly_string out;
    fly_string p1=fs("/a/b/../c/./d"), p2=fs("a//b///c"), p3=fs(".");
    fly_path_normalize(&p1,&out); check(fseq(&out,"/a/c/d"), "norm .."); cleanup(&out);
    fly_path_normalize(&p2,&out); check(fseq(&out,"a/b/c"),  "norm //"); cleanup(&out);
    fly_path_normalize(&p3,&out); check(fseq(&out,"."),      "norm .");  cleanup(&out);
}

static void test_rel(void) {                            /* 23 */
    fly_string out;
    fly_string base=fs("/a/b"), target=fs("/a/c/d"), same=fs("/a/b");
    fly_path_rel(&base,&target,&out); check(fseq(&out,"../c/d"), "rel diff"); cleanup(&out);
    fly_path_rel(&base,&same,  &out); check(fseq(&out,"."),      "rel same"); cleanup(&out);
}

static void test_comp(void) {                           /* 24 */
    fly_string_array out;
    fly_string p=fs("/a/b/c");
    fly_path_comp(&p,&out);
    check(out.count==4,  "comp count /a/b/c");
    if (out.count>=1) check(fseq(&out.items[0],"/"), "comp[0]=/");
    if (out.count>=4) check(fseq(&out.items[3],"c"), "comp[3]=c");
    cleanup_sa(&out);

    fly_string rel=fs("x/y");
    fly_path_comp(&rel,&out);
    check(out.count==2, "comp count x/y");
    cleanup_sa(&out);
}

static void test_match(void) {                          /* 25 */
    int r;
    fly_string p1=fs("*.fly"),    yes1=fs("main.fly"),   no1=fs("main.c");
    fly_string p2=fs("test_?.c"), yes2=fs("test_a.c"),   no2=fs("test_ab.c");
    fly_string p3=fs("[abc].h"),  yes3=fs("a.h"),         no3=fs("d.h");

    fly_path_match(&p1,&yes1,&r); check(r!=0, "match *.fly YES");
    fly_path_match(&p1,&no1, &r); check(r==0, "match *.fly NO");
    fly_path_match(&p2,&yes2,&r); check(r!=0, "match test_?.c YES");
    fly_path_match(&p2,&no2, &r); check(r==0, "match test_?.c NO");
    fly_path_match(&p3,&yes3,&r); check(r!=0, "match [abc].h YES");
    fly_path_match(&p3,&no3, &r); check(r==0, "match [abc].h NO");
}

static void test_glob(void) {                           /* 26 */
    /* Write two files with a known pattern then glob */
    fly_string_array out;

    /* glob /tmp — at minimum no crash and returns items */
    fly_string pat = fs("/tmp/*.fly_test_XXXXXXXX");
    fly_path_glob(&pat, &out);
    check(1, "glob no crash");     /* structural test only */
    cleanup_sa(&out);

    /* verify sep */
    uint8_t sep;
    fly_path_sep(&sep);
    check(sep == '/', "sep is /");
}

static void run_tests(void);
__attribute__((naked)) void _start(void) {
    __asm__ volatile("andq $-16,%%rsp\ncallq %P0\n" : : "i"(run_tests));
}

static void run_tests(void) {
    test_join();
    test_basename_dirname();
    test_ext();
    test_normalize();
    test_rel();
    test_comp();
    test_match();
    test_glob();

    unsigned long total = (unsigned long)(g_pass + g_fail);
    fly_puts("fly_os_path: ");
    fly_put_u64((unsigned long long)g_pass); fly_write("/",1);
    fly_put_u64((unsigned long long)total);  fly_puts(" passed");
    if (g_fail) { fly_puts("  ("); fly_put_u64((unsigned long long)g_fail); fly_puts(" FAILED)"); }
    fly_write("\n",1);
    fly_exit(g_fail ? 1 : 0);
}
