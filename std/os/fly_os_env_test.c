/*===-- std/os/fly_os_env_test.c - freestanding tests for fly_os_env ----===*/

#include "fly_os_env.h"

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
static int g_pass=0, g_fail=0;
static void check(int cond, const char *name) {
    if (cond) g_pass++;
    else { g_fail++; fly_puts("FAIL: "); fly_puts(name); fly_write("\n",1); }
}

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static fly_string fs(const char *s) {
    fly_string r; int n=0; while(s[n])n++;
    r.ptr=(char*)s; r.size=n; return r;
}
static int fseq(const fly_string *a, const char *b) {
    int n=0; while(b[n])n++;
    if (a->size!=n) return 0;
    for (int i=0;i<n;i++) if(a->ptr[i]!=b[i]) return 0;
    return 1;
}
static int fseq2(const fly_string *a, const fly_string *b) {
    if (a->size!=b->size) return 0;
    for (int i=0;i<a->size;i++) if(a->ptr[i]!=b->ptr[i]) return 0;
    return 1;
}
static void cleanup(fly_string *s) {
    if (s->ptr){free(s->ptr);s->ptr=(char*)0;s->size=0;}
}
static void cleanup_sa(fly_string_array *a) {
    if (a->items) {
        for (int i=0;i<a->count;i++) if(a->items[i].ptr) free(a->items[i].ptr);
        free(a->items); a->items=(fly_string*)0; a->count=0;
    }
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_set_get(void) {                    /* 27 */
    fly_string key=fs("FLY_TEST_VAR"), val=fs("hello_fly");
    fly_env_set(&key, &val);
    fly_string out;
    fly_env_get(&key, &out);
    check(fseq(&out, "hello_fly"), "env set+get round-trip");
    cleanup(&out);
}

static void test_delete(void) {                     /* 28 */
    fly_string key=fs("FLY_TEST_DELETE"), val=fs("to_remove");
    fly_env_set(&key, &val);
    fly_env_delete(&key);
    fly_string out;
    fly_env_get(&key, &out);
    check(out.size==0, "env delete → empty"); cleanup(&out);
}

static void test_all(void) {                        /* 29 */
    fly_string key=fs("FLY_TEST_ALL"), val=fs("visible");
    fly_env_set(&key, &val);
    fly_string_array all;
    fly_env_all(&all);
    int found = 0;
    for (int i=0; i<all.count; i++) {
        fly_string *e = &all.items[i];
        if (e->size >= 15) {
            int match = 1;
            const char *kv = "FLY_TEST_ALL=visible";
            int klen = 0; while(kv[klen])klen++;
            if (e->size < klen) match=0;
            else for (int j=0;j<klen;j++) if(e->ptr[j]!=kv[j]){match=0;break;}
            if (match) found=1;
        }
    }
    check(found, "env all contains set var");
    cleanup_sa(&all);
}

static void test_expand(void) {                     /* 30 */
    fly_string key=fs("FLY_HOME"), val=fs("/home/fly");
    fly_env_set(&key, &val);
    fly_string templ = fs("$FLY_HOME/subdir");
    fly_string out;
    fly_env_expand(&templ, &out);
    check(fseq(&out, "/home/fly/subdir"), "expand $VAR/subdir"); cleanup(&out);

    fly_string templ2 = fs("${FLY_HOME}/other");
    fly_env_expand(&templ2, &out);
    check(fseq(&out, "/home/fly/other"), "expand ${VAR}/other"); cleanup(&out);
}

static void test_cwd(void) {                        /* 31 */
    fly_string orig;
    fly_env_cwdGet(&orig);
    check(orig.size > 0, "cwdGet non-empty");

    fly_string tmp = fs("/tmp");
    fly_env_cwdSet(&tmp);
    fly_string after;
    fly_env_cwdGet(&after);
    check(fseq(&after, "/tmp"), "cwdSet /tmp"); cleanup(&after);

    /* restore */
    fly_env_cwdSet(&orig);
    fly_string restored;
    fly_env_cwdGet(&restored);
    check(fseq2(&restored, &orig), "cwd restored");
    check(restored.size == orig.size, "cwd restored size");
    cleanup(&restored);
    cleanup(&orig);
}

static void test_args(void) {                       /* 32 */
    fly_string_array args;
    fly_env_argsGet(&args);
    check(args.count >= 1, "argsGet count >= 1");
    if (args.count >= 1) check(args.items[0].size > 0, "argv[0] non-empty");
    cleanup_sa(&args);

    int count;
    fly_env_argsCount(&count);
    check(count >= 1, "argsCount >= 1");
}

static void test_hostname(void) {                   /* 33 */
    fly_string out;
    fly_env_hostname(&out);
    check(out.size > 0, "hostname non-empty"); cleanup(&out);
}

static void test_osname(void) {                     /* 34 */
    fly_string out;
    fly_env_osname(&out);
    check(fseq(&out, "linux"), "osname=linux"); cleanup(&out);
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

static void run_tests(void);

/* Capture argc/argv from kernel stack before aligning */
static void entry(long *sp) {
    int argc = (int)sp[0];
    char **argv = (char **)(sp + 1);
    fly_env_init(argc, argv);
    run_tests();
}

__attribute__((naked)) void _start(void) {
    __asm__ volatile(
        "movq  %%rsp, %%rdi\n"
        "andq  $-16,  %%rsp\n"
        "callq %P0\n"
        : : "i"(entry)
    );
}

static void run_tests(void) {
    test_set_get();
    test_delete();
    test_all();
    test_expand();
    test_cwd();
    test_args();
    test_hostname();
    test_osname();

    unsigned long total = (unsigned long)(g_pass + g_fail);
    fly_puts("fly_os_env: ");
    fly_put_u64((unsigned long long)g_pass); fly_write("/",1);
    fly_put_u64((unsigned long long)total);  fly_puts(" passed");
    if (g_fail) { fly_puts("  ("); fly_put_u64((unsigned long long)g_fail); fly_puts(" FAILED)"); }
    fly_write("\n",1);
    fly_exit(g_fail ? 1 : 0);
}
