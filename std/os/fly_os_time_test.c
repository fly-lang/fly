/*===-- std/os/fly_os_time_test.c - freestanding tests for fly_os_time --===*/

#include "fly_os_time.h"

extern void free(void *);

/* ── Freestanding test harness ───────────────────────────────────────────── */

static __attribute__((noinline)) void fly_write(const char *buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"(1L), "S"(buf), "d"(len)
        : "rcx", "r11", "memory");
}
static void fly_puts(const char *s) { long n=0; while(s[n])n++; fly_write(s,n); }
static void fly_put_u64(unsigned long long v) {
    if (!v){fly_write("0",1);return;}
    char buf[20]; int len=0;
    while(v){buf[len++]=(char)('0'+(int)(v%10));v/=10;}
    for(int lo=0,hi=len-1;lo<hi;lo++,hi--){char t=buf[lo];buf[lo]=buf[hi];buf[hi]=t;}
    fly_write(buf,(long)len);
}
static __attribute__((noreturn,noinline)) void fly_exit(int code) {
    __asm__ volatile("syscall"::  "a"(60L),"D"((long)code):"rcx","r11","memory");
    __builtin_unreachable();
}
static int g_pass=0,g_fail=0;
static void check(int cond,const char *name){
    if(cond)g_pass++;
    else{g_fail++;fly_puts("FAIL: ");fly_puts(name);fly_write("\n",1);}
}

static fly_string fs(const char *s){
    fly_string r; int n=0; while(s[n])n++;
    r.ptr=(char*)s; r.size=n; return r;
}
static int fseq(const fly_string *a,const char *b){
    int n=0; while(b[n])n++;
    if(a->size!=n)return 0;
    for(int i=0;i<n;i++)if(a->ptr[i]!=b[i])return 0;
    return 1;
}
static void cleanup(fly_string *s){
    if(s->ptr){free(s->ptr);s->ptr=(char*)0;s->size=0;}
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static void test_now(void) {                        /* 35 */
    fly_time t;
    fly_time_now(&t);
    check(t.sec > 0, "now sec > 0");
    check(t.nsec >= 0 && t.nsec < FLY_SECOND, "now nsec in range");
}

static void test_monotonic(void) {                  /* 36 */
    fly_time a, b;
    fly_time_monotonic(&a);
    /* minimal busy-wait — just two reads */
    fly_time_monotonic(&b);
    int cmp;
    fly_time_compare(&a, &b, &cmp);
    check(cmp <= 0, "monotonic non-decreasing");
}

static void test_sleep(void) {                      /* 37 */
    fly_time before;
    fly_time_monotonic(&before);
    fly_duration d; d.nsec = 10 * FLY_MILLISECOND;
    fly_time_sleep(&d);
    fly_duration elapsed;
    fly_time_since(&before, &elapsed);
    check(elapsed.nsec >= 10 * FLY_MILLISECOND, "sleep >= 10ms");
}

static void test_diff(void) {                       /* 38 */
    fly_time a, b;
    a.sec = 100; a.nsec = 500000000LL;
    b.sec = 101; b.nsec = 200000000LL;
    fly_duration d;
    fly_time_diff(&a, &b, &d);
    /* b - a = 0.7s = 700_000_000 ns */
    check(d.nsec == 700000000LL, "diff correct");
}

static void test_add(void) {                        /* 39 */
    fly_time t, out;
    t.sec  = 1000;
    t.nsec = 0;
    fly_duration d; d.nsec = FLY_SECOND;
    fly_time_add(&t, &d, &out);
    check(out.sec == 1001 && out.nsec == 0, "add +1s");

    fly_duration d2; d2.nsec = FLY_MILLISECOND * 500;
    fly_time_add(&t, &d2, &out);
    check(out.sec == 1000 && out.nsec == 500000000LL, "add +500ms");
}

static void test_unix_round_trip(void) {            /* 40 */
    fly_time t, out;
    t.sec = 1700000000LL; t.nsec = 0;
    int64_t unix_sec;
    fly_time_unix(&t, &unix_sec);
    check(unix_sec == 1700000000LL, "unix seconds");

    fly_time_fromUnix(1700000000LL, &out);
    check(out.sec == 1700000000LL && out.nsec == 0, "fromUnix");
}

static void test_format(void) {                     /* 41 */
    fly_time t;
    /* 2024-01-15 — known unix timestamp: 1705276800 = 2024-01-15 00:00:00 UTC */
    fly_time_fromUnix(1705276800LL, &t);
    fly_string pat = fs("2006-01-02");
    fly_string out;
    fly_time_format(&t, &pat, &out);
    check(fseq(&out, "2024-01-15"), "format 2006-01-02"); cleanup(&out);
}

static void test_parse_format_roundtrip(void) {     /* 42 */
    fly_string s  = fs("2024-03-07");
    fly_string pat = fs("2006-01-02");
    fly_time parsed;
    fly_time_parse(&s, &pat, &parsed);
    fly_string out;
    fly_time_format(&parsed, &pat, &out);
    check(fseq(&out, "2024-03-07"), "parse+format round-trip"); cleanup(&out);
}

static void test_duration_format(void) {            /* 43 */
    fly_duration d0;  d0.nsec  = 0;
    fly_duration dh;  dh.nsec  = FLY_HOUR + 30*FLY_MINUTE;
    fly_duration dms; dms.nsec = 200*FLY_MILLISECOND;
    fly_string out;

    fly_time_durationFormat(&d0,  &out); check(fseq(&out,"0s"),    "dur 0");   cleanup(&out);
    fly_time_durationFormat(&dms, &out); check(fseq(&out,"200ms"), "dur 200ms"); cleanup(&out);
    fly_time_durationFormat(&dh,  &out);
    /* "1h30m00s" */
    check(out.size > 0, "dur 1h30m non-empty"); cleanup(&out);

    int64_t secs, millis, micros;
    fly_duration d1s; d1s.nsec = 3 * FLY_SECOND + 500*FLY_MILLISECOND;
    fly_time_durationSecs  (&d1s, &secs);   check(secs   == 3, "durationSecs");
    fly_time_durationMillis(&d1s, &millis); check(millis == 3500, "durationMillis");
    fly_time_durationMicros(&d1s, &micros); check(micros == 3500000LL, "durationMicros");
}

static void run_tests(void);
__attribute__((naked)) void _start(void) {
    __asm__ volatile("andq $-16,%%rsp\ncallq %P0\n"::"i"(run_tests));
}
static void run_tests(void) {
    test_now();
    test_monotonic();
    test_sleep();
    test_diff();
    test_add();
    test_unix_round_trip();
    test_format();
    test_parse_format_roundtrip();
    test_duration_format();

    unsigned long total=(unsigned long)(g_pass+g_fail);
    fly_puts("fly_os_time: ");
    fly_put_u64((unsigned long long)g_pass); fly_write("/",1);
    fly_put_u64((unsigned long long)total);  fly_puts(" passed");
    if(g_fail){fly_puts("  (");fly_put_u64((unsigned long long)g_fail);fly_puts(" FAILED)");}
    fly_write("\n",1);
    fly_exit(g_fail?1:0);
}
