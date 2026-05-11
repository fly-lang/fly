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
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &path, &data, 0644);
    fly_buf out = {(uint8_t*)0,0,0};
    _F6fly_os6fsRead_Ss_Cfly_buf((void*)0, &path, &out);
    check(fseq_buf(&out,"hello world"), "create+write+read"); cleanup_b(&out);
    _F6fly_os8fsDelete_Ss((void*)0, &path);
}

static void test_append(void) {                     /* 6 */
    fly_string path = fs("/tmp/fly_fs_test_app.txt");
    fly_buf d1; d1.ptr=(uint8_t*)"foo"; d1.size=3; d1.cap=3;
    fly_buf d2; d2.ptr=(uint8_t*)"bar"; d2.size=3; d2.cap=3;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &path, &d1, 0644);
    _F6fly_os8fsAppend_Ss_Cfly_buf_ui((void*)0, &path, &d2, 0644);
    fly_buf out={0,0,0};
    _F6fly_os6fsRead_Ss_Cfly_buf((void*)0, &path, &out);
    check(fseq_buf(&out,"foobar"), "append concat"); cleanup_b(&out);
    _F6fly_os8fsDelete_Ss((void*)0, &path);
}

static void test_seek(void) {                       /* 7 */
    fly_string path = fs("/tmp/fly_fs_test_seek.txt");
    fly_buf d; d.ptr=(uint8_t*)"0123456789"; d.size=10; d.cap=10;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &path, &d, 0644);
    fly_file f;
    _F6fly_os6fsOpen_Ss_Cfly_file((void*)0, &path, &f);
    check(f.fd >= 0, "seek open ok");
    int64_t pos;
    _F6fly_os9fsSeekTo_Cfly_file_l_i_l((void*)0, &f, 5, FLY_SEEK_SET, &pos); check(pos==5, "seekTo SET=5");
    _F6fly_os9fsSeekTo_Cfly_file_l_i_l((void*)0, &f, 2, FLY_SEEK_CUR, &pos); check(pos==7, "seekTo CUR+2=7");
    _F6fly_os9fsSeekTo_Cfly_file_l_i_l((void*)0, &f,-3, FLY_SEEK_END, &pos); check(pos==7, "seekTo END-3=7");
    _F6fly_os9fsSeekPos_Cfly_file_l((void*)0, &f, &pos);                     check(pos==7, "seekPos=7");
    _F6fly_os7fsClose_Cfly_file((void*)0, &f);
    _F6fly_os8fsDelete_Ss((void*)0, &path);
}

static void test_stat(void) {                       /* 8 */
    fly_string path = fs("/tmp/fly_fs_test_stat.txt");
    fly_buf d; d.ptr=(uint8_t*)"stat test"; d.size=9; d.cap=9;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &path, &d, 0644);
    fly_stat st;
    _F6fly_os6fsStat_Ss_Cfly_stat((void*)0, &path, &st);
    check(st.size == 9,   "stat size=9");
    check(st.is_file != 0,"stat is_file");
    check(st.is_dir == 0, "stat not is_dir");
    _F6fly_os8fsDelete_Ss((void*)0, &path);
}

static void test_exists(void) {                     /* 9 */
    fly_string yes = fs("/tmp");
    fly_string no  = fs("/tmp/fly_THIS_DOES_NOT_EXIST_123456");
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &yes, &r); check(r!=0, "exists /tmp");
    _F6fly_os8fsExists_Ss_b((void*)0, &no,  &r); check(r==0, "not exists");
}

static void test_copy(void) {                       /* 10 */
    fly_string src = fs("/tmp/fly_fs_test_src.txt");
    fly_string dst = fs("/tmp/fly_fs_test_dst.txt");
    fly_buf d; d.ptr=(uint8_t*)"copydata"; d.size=8; d.cap=8;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &src, &d, 0644);
    _F6fly_os6fsCopy_Ss_Ss((void*)0, &src, &dst);
    fly_buf out={0,0,0};
    _F6fly_os6fsRead_Ss_Cfly_buf((void*)0, &dst, &out);
    check(fseq_buf(&out,"copydata"), "copy content"); cleanup_b(&out);
    _F6fly_os8fsDelete_Ss((void*)0, &src);
    _F6fly_os8fsDelete_Ss((void*)0, &dst);
}

static void test_move(void) {                       /* 11 */
    fly_string src = fs("/tmp/fly_fs_test_move_src.txt");
    fly_string dst = fs("/tmp/fly_fs_test_move_dst.txt");
    fly_buf d; d.ptr=(uint8_t*)"moved"; d.size=5; d.cap=5;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &src, &d, 0644);
    _F6fly_os6fsMove_Ss_Ss((void*)0, &src, &dst);
    int r_src, r_dst;
    _F6fly_os8fsExists_Ss_b((void*)0, &src, &r_src);
    _F6fly_os8fsExists_Ss_b((void*)0, &dst, &r_dst);
    check(r_src==0, "move: src gone");
    check(r_dst!=0, "move: dst exists");
    _F6fly_os8fsDelete_Ss((void*)0, &dst);
}

static void test_delete(void) {                     /* 12 */
    fly_string path = fs("/tmp/fly_fs_test_del.txt");
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &path, &d, 0644);
    _F6fly_os8fsDelete_Ss((void*)0, &path);
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &path, &r);
    check(r==0, "delete → not exists");
}

static void test_dir_create_read(void) {            /* 13 */
    fly_string dir  = fs("/tmp/fly_fs_test_dir");
    fly_string file = fs("/tmp/fly_fs_test_dir/f.txt");
    _F6fly_os11fsDirCreate_Ss_ui((void*)0, &dir, 0755);
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &file, &d, 0644);
    fly_dir_entries ents = {0,0,0};
    _F6fly_os9fsDirRead_Ss_Cfly_dir_entries((void*)0, &dir, &ents);
    check(ents.len >= 1, "dirRead has entry");
    cleanup_de(&ents);
    _F6fly_os8fsDelete_Ss((void*)0, &file);
    _F6fly_os11fsDirDelete_Ss((void*)0, &dir);
}

static void test_dir_create_all(void) {             /* 14 */
    fly_string path = fs("/tmp/fly_fs_test_nested/a/b/c");
    _F6fly_os14fsDirCreateAll_Ss_ui((void*)0, &path, 0755);
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &path, &r);
    check(r!=0, "dirCreateAll nested");
    /* cleanup */
    fly_string p3 = fs("/tmp/fly_fs_test_nested/a/b/c");
    fly_string p2 = fs("/tmp/fly_fs_test_nested/a/b");
    fly_string p1 = fs("/tmp/fly_fs_test_nested/a");
    fly_string p0 = fs("/tmp/fly_fs_test_nested");
    _F6fly_os11fsDirDelete_Ss((void*)0, &p3);
    _F6fly_os11fsDirDelete_Ss((void*)0, &p2);
    _F6fly_os11fsDirDelete_Ss((void*)0, &p1);
    _F6fly_os11fsDirDelete_Ss((void*)0, &p0);
}

static void test_dir_delete_all(void) {             /* 15 */
    fly_string root = fs("/tmp/fly_fs_test_rmdirall");
    _F6fly_os14fsDirCreateAll_Ss_ui((void*)0, &root, 0755);
    fly_string sub = fs("/tmp/fly_fs_test_rmdirall/sub");
    _F6fly_os11fsDirCreate_Ss_ui((void*)0, &sub, 0755);
    fly_string f = fs("/tmp/fly_fs_test_rmdirall/sub/f.txt");
    fly_buf d; d.ptr=(uint8_t*)"x"; d.size=1; d.cap=1;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &f, &d, 0644);
    _F6fly_os14fsDirDeleteAll_Ss((void*)0, &root);
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &root, &r);
    check(r==0, "dirDeleteAll removes tree");
}

static void test_symlink(void) {                    /* 16 */
    fly_string target = fs("/tmp/fly_fs_test_sym_target.txt");
    fly_string link   = fs("/tmp/fly_fs_test_sym_link.lnk");
    fly_buf d; d.ptr=(uint8_t*)"sym"; d.size=3; d.cap=3;
    _F6fly_os7fsWrite_Ss_Cfly_buf_ui((void*)0, &target, &d, 0644);
    _F6fly_os15fsSymlinkCreate_Ss_Ss((void*)0, &target, &link);
    fly_string dest;
    _F6fly_os13fsSymlinkRead_Ss_Ss((void*)0, &link, &dest);
    int n=0; const char *tv="/tmp/fly_fs_test_sym_target.txt";
    while(tv[n])n++;
    check(dest.size==n && dest.ptr, "symlinkRead round-trip"); cleanup_s(&dest);
    _F6fly_os8fsDelete_Ss((void*)0, &link);
    _F6fly_os8fsDelete_Ss((void*)0, &target);
}

static void test_temp_file(void) {                  /* 17 */
    fly_string dir = fs("/tmp"), pat = fs("fly_tmp_");
    fly_string out_path; fly_file out_file;
    _F6fly_os10fsTempFile_Ss_Ss_Ss_Cfly_file((void*)0, &dir, &pat, &out_path, &out_file);
    check(out_file.fd >= 0, "tempFile fd open");
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &out_path, &r);
    check(r!=0, "tempFile exists");
    _F6fly_os7fsClose_Cfly_file((void*)0, &out_file);
    _F6fly_os8fsDelete_Ss((void*)0, &out_path);
    cleanup_s(&out_path);
}

static void test_temp_dir(void) {                   /* 18 */
    fly_string dir=fs("/tmp"), pat=fs("fly_tmpdir_");
    fly_string out;
    _F6fly_os9fsTempDir_Ss_Ss_Ss((void*)0, &dir, &pat, &out);
    int r;
    _F6fly_os8fsExists_Ss_b((void*)0, &out, &r);
    check(r!=0, "tempDir exists");
    _F6fly_os11fsDirDelete_Ss((void*)0, &out);
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
