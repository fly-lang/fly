/*===-- std/os/fly_os_fs_test.c - freestanding tests for fly_os_fs ------===*/

#include "fly_os_fs.h"

extern void free(void *);

/* ── Test harness ────────────────────────────────────────────────────────── */

static __attribute__((noinline)) void fly_write(const char *buf, long len) {
    __asm__ volatile("syscall"
        : : "a"(1L), "D"(1L), "S"(buf), "d"(len) : "rcx", "r11", "memory");
}
static void fly_puts(const char *s){long n=0;while(s[n])n++;fly_write(s,n);}
static void fly_put_u64(unsigned long long v){
    if(!v){fly_write("0",1);return;}
    char buf[20];int len=0;
    while(v){buf[len++]=(char)('0'+(int)(v%10));v/=10;}
    for(int lo=0,hi=len-1;lo<hi;lo++,hi--){char t=buf[lo];buf[lo]=buf[hi];buf[hi]=t;}
    fly_write(buf,(long)len);
}
static __attribute__((noreturn,noinline)) void fly_exit(int code){
    __asm__ volatile("syscall"::  "a"(60L),"D"((long)code):"rcx","r11","memory");
    __builtin_unreachable();
}
static int g_pass=0,g_fail=0;
static void check(int cond,const char *name){
    if(cond)g_pass++;
    else{g_fail++;fly_puts("FAIL: ");fly_puts(name);fly_write("\n",1);}
}

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static fly_string fs(const char *s){
    fly_string r;int n=0;while(s[n])n++;
    r.ptr=(char*)s;r.size=n;return r;
}
static int fseq_buf(const fly_buf *b, const char *s){
    int n=0;while(s[n])n++;
    if((int)b->size!=n)return 0;
    for(int i=0;i<n;i++) if((char)b->ptr[i]!=s[i])return 0;
    return 1;
}
static void cleanup_s(fly_string *s){
    if(s->ptr){free(s->ptr);s->ptr=(char*)0;s->size=0;}
}
static void cleanup_b(fly_buf *b){
    if(b->ptr){free(b->ptr);b->ptr=(uint8_t*)0;b->size=0;b->cap=0;}
}
static void cleanup_de(fly_dir_entries *e){
    if(e->items){
        for(size_t i=0;i<e->len;i++) if(e->items[i].name.ptr) free(e->items[i].name.ptr);
        free(e->items);e->items=(fly_dir_entry*)0;e->len=0;e->cap=0;
    }
}

/* ── Tests ─────────────────────────────────────────────────────────────────── */

static const char *TMPDIR = "/tmp/fly_fs_test";

static void test_create_write_read(void) {          /* 5 */
    fly_string path = fs("/tmp/fly_fs_test_rw.txt");
    fly_buf data;
    data.ptr  = (uint8_t *)"hello world";
    data.size = 11;
    data.cap  = 11;
    fly_fs_write(&path, &data, 0644);
    fly_buf out = {(uint8_t*)0,0,0};
    fly_fs_read(&path, &out);
    check(fseq_buf(&out,"hello world"), "create+write+read"); cleanup_b(&out);
    fly_fs_delete(&path);
}

static void test_append(void) {                     /* 6 */
    fly_string path = fs("/tmp/fly_fs_test_app.txt");
    fly_buf d1; d1.ptr=(uint8_t*)"foo"; d1.size=3; d1.cap=3;
    fly_buf d2; d2.ptr=(uint8_t*)"bar"; d2.size=3; d2.cap=3;
    fly_fs_write(&path, &d1, 0644);
    fly_fs_append(&path, &d2, 0644);
    fly_buf out={0,0,0};
    fly_fs_read(&path, &out);
    check(fseq_buf(&out,"foobar"), "append concat"); cleanup_b(&out);
    fly_fs_delete(&path);
}

static void test_seek(void) {                       /* 7 */
    fly_string path = fs("/tmp/fly_fs_test_seek.txt");
    fly_buf d; d.ptr=(uint8_t*)"0123456789"; d.size=10; d.cap=10;
    fly_fs_write(&path, &d, 0644);
    fly_file f; fly_fs_open(&path, &f);
    check(f.fd >= 0, "seek open ok");
    int64_t pos;
    fly_fs_seekTo(&f, 5, FLY_SEEK_SET, &pos); check(pos==5, "seekTo SET=5");
    fly_fs_seekTo(&f, 2, FLY_SEEK_CUR, &pos); check(pos==7, "seekTo CUR+2=7");
    fly_fs_seekTo(&f,-3, FLY_SEEK_END, &pos); check(pos==7, "seekTo END-3=7");
    fly_fs_seekPos(&f, &pos);                  check(pos==7, "seekPos=7");
    fly_fs_close(&f);
    fly_fs_delete(&path);
}

static void test_stat(void) {                       /* 8 */
    fly_string path = fs("/tmp/fly_fs_test_stat.txt");
    fly_buf d; d.ptr=(uint8_t*)"stat test"; d.size=9; d.cap=9;
    fly_fs_write(&path, &d, 0644);
    fly_stat st;
    fly_fs_stat(&path, &st);
    check(st.size == 9,   "stat size=9");
    check(st.is_file != 0,"stat is_file");
    check(st.is_dir == 0, "stat not is_dir");
    fly_fs_delete(&path);
}

static void test_exists(void) {                     /* 9 */
    fly_string yes = fs("/tmp");
    fly_string no  = fs("/tmp/fly_THIS_DOES_NOT_EXIST_123456");
    int r;
    fly_fs_exists(&yes, &r); check(r!=0, "exists /tmp");
    fly_fs_exists(&no,  &r); check(r==0, "not exists");
}

static void test_copy(void) {                       /* 10 */
    fly_string src = fs("/tmp/fly_fs_test_src.txt");
    fly_string dst = fs("/tmp/fly_fs_test_dst.txt");
    fly_buf d; d.ptr=(uint8_t*)"copydata"; d.size=8; d.cap=8;
    fly_fs_write(&src, &d, 0644);
    fly_fs_copy(&src, &dst);
    fly_buf out={0,0,0};
    fly_fs_read(&dst, &out);
    check(fseq_buf(&out,"copydata"), "copy content"); cleanup_b(&out);
    fly_fs_delete(&src); fly_fs_delete(&dst);
}

static void test_move(void) {                       /* 11 */
    fly_string src = fs("/tmp/fly_fs_test_move_src.txt");
    fly_string dst = fs("/tmp/fly_fs_test_move_dst.txt");
    fly_buf d; d.ptr=(uint8_t*)"moved"; d.size=5; d.cap=5;
    fly_fs_write(&src, &d, 0644);
    fly_fs_move(&src, &dst);
    int r_src, r_dst;
    fly_fs_exists(&src, &r_src);
    fly_fs_exists(&dst, &r_dst);
    check(r_src==0, "move: src gone");
    check(r_dst!=0, "move: dst exists");
    fly_fs_delete(&dst);
}

static void test_delete(void) {                     /* 12 */
    fly_string path = fs("/tmp/fly_fs_test_del.txt");
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    fly_fs_write(&path, &d, 0644);
    fly_fs_delete(&path);
    int r; fly_fs_exists(&path, &r);
    check(r==0, "delete → not exists");
}

static void test_dir_create_read(void) {            /* 13 */
    fly_string dir  = fs("/tmp/fly_fs_test_dir");
    fly_string file = fs("/tmp/fly_fs_test_dir/f.txt");
    fly_fs_dirCreate(&dir, 0755);
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    fly_fs_write(&file, &d, 0644);
    fly_dir_entries ents = {0,0,0};
    fly_fs_dirRead(&dir, &ents);
    check(ents.len >= 1, "dirRead has entry");
    cleanup_de(&ents);
    fly_fs_delete(&file);
    fly_fs_dirDelete(&dir);
}

static void test_dir_create_all(void) {             /* 14 */
    fly_string path = fs("/tmp/fly_fs_test_nested/a/b/c");
    fly_fs_dirCreateAll(&path, 0755);
    int r; fly_fs_exists(&path, &r);
    check(r!=0, "dirCreateAll nested");
    /* cleanup */
    fly_string p3 = fs("/tmp/fly_fs_test_nested/a/b/c");
    fly_string p2 = fs("/tmp/fly_fs_test_nested/a/b");
    fly_string p1 = fs("/tmp/fly_fs_test_nested/a");
    fly_string p0 = fs("/tmp/fly_fs_test_nested");
    fly_fs_dirDelete(&p3); fly_fs_dirDelete(&p2);
    fly_fs_dirDelete(&p1); fly_fs_dirDelete(&p0);
}

static void test_dir_delete_all(void) {             /* 15 */
    fly_string root = fs("/tmp/fly_fs_test_rmdirall");
    fly_fs_dirCreateAll(&root, 0755);
    fly_string sub = fs("/tmp/fly_fs_test_rmdirall/sub");
    fly_fs_dirCreate(&sub, 0755);
    fly_string f = fs("/tmp/fly_fs_test_rmdirall/sub/f.txt");
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    fly_fs_write(&f, &d, 0644);
    fly_fs_dirDeleteAll(&root);
    int r; fly_fs_exists(&root, &r);
    check(r==0, "dirDeleteAll removes tree");
}

static void test_symlink(void) {                    /* 16 */
    fly_string target = fs("/tmp/fly_fs_test_sym_target.txt");
    fly_string link   = fs("/tmp/fly_fs_test_sym_link.lnk");
    fly_buf d; d.ptr=(uint8_t*)"sym"; d.size=3; d.cap=3;
    fly_fs_write(&target, &d, 0644);
    fly_fs_symlinkCreate(&target, &link);
    fly_string dest;
    fly_fs_symlinkRead(&link, &dest);
    int n=0; const char *tv="/tmp/fly_fs_test_sym_target.txt";
    while(tv[n])n++;
    check(dest.size==n && dest.ptr, "symlinkRead round-trip"); cleanup_s(&dest);
    fly_fs_delete(&link); fly_fs_delete(&target);
}

static void test_temp_file(void) {                  /* 17 */
    fly_string dir = fs("/tmp"), pat = fs("fly_tmp_");
    fly_string out_path; fly_file out_file;
    fly_fs_tempFile(&dir, &pat, &out_path, &out_file);
    check(out_file.fd >= 0, "tempFile fd open");
    int r; fly_fs_exists(&out_path, &r);
    check(r!=0, "tempFile exists");
    fly_fs_close(&out_file);
    fly_fs_delete(&out_path);
    cleanup_s(&out_path);
}

static void test_temp_dir(void) {                   /* 18 */
    fly_string dir=fs("/tmp"), pat=fs("fly_tmpdir_");
    fly_string out;
    fly_fs_tempDir(&dir, &pat, &out);
    int r; fly_fs_exists(&out, &r);
    check(r!=0, "tempDir exists");
    fly_fs_dirDelete(&out);
    cleanup_s(&out);
}

static void run_tests(void);
__attribute__((naked)) void _start(void) {
    __asm__ volatile("andq $-16,%%rsp\ncallq %P0\n"::"i"(run_tests));
}
static void run_tests(void) {
    test_create_write_read();
    test_append();
    test_seek();
    test_stat();
    test_exists();
    test_copy();
    test_move();
    test_delete();
    test_dir_create_read();
    test_dir_create_all();
    test_dir_delete_all();
    test_symlink();
    test_temp_file();
    test_temp_dir();

    unsigned long total=(unsigned long)(g_pass+g_fail);
    fly_puts("fly_os_fs: ");
    fly_put_u64((unsigned long long)g_pass);fly_write("/",1);
    fly_put_u64((unsigned long long)total);fly_puts(" passed");
    if(g_fail){fly_puts("  (");fly_put_u64((unsigned long long)g_fail);fly_puts(" FAILED)");}
    fly_write("\n",1);
    fly_exit(g_fail?1:0);
}
