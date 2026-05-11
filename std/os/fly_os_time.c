/*===-- std/os/fly_os_time.c - fly.os time implementation ---------------===*/

#include "fly_os_time.h"
#include "fly_os_linux.h"

extern void *malloc(unsigned long);
extern void  free  (void *);

/* ── Internal helpers ────────────────────────────────────────────────────── */

static void fstr_from_cstr(const char *s, int len, fly_string *out) {
    if (len <= 0) { out->ptr = (char *)0; out->size = 0; return; }
    char *p = (char *)malloc((unsigned long)len);
    for (int i = 0; i < len; i++) p[i] = s[i];
    out->ptr  = p;
    out->size = len;
}

/* int64 absolute value */
static int64_t i64abs(int64_t v) { return v < 0 ? -v : v; }

/* ── Calendar arithmetic ─────────────────────────────────────────────────── *
 * Algorithm by Howard Hinnant (public domain):
 *   http://howardhinnant.github.io/date_algorithms.html
 *===----------------------------------------------------------------------- */

static void unix_to_ymd(int64_t sec, int *year, int *month, int *day) {
    long z = (long)(sec / 86400);
    if (sec < 0 && sec % 86400 != 0) z--;
    z += 719468L;
    long era = (z >= 0 ? z : z - 146096L) / 146097L;
    long doe = z - era * 146097L;
    long yoe = (doe - doe/1460L + doe/36524L - doe/146096L) / 365L;
    long y   = yoe + era * 400L;
    long doy = doe - (365L*yoe + yoe/4L - yoe/100L);
    long mp  = (5L*doy + 2L) / 153L;
    long d   = doy - (153L*mp + 2L)/5L + 1L;
    long m   = mp < 10L ? mp + 3L : mp - 9L;
    y += (m <= 2L);
    *year  = (int)y;
    *month = (int)m;
    *day   = (int)d;
}

static void unix_to_hms(int64_t sec, int *h, int *m, int *s) {
    long sod = (long)(sec % 86400L);
    if (sod < 0) sod += 86400L;
    *h = (int)(sod / 3600L);
    *m = (int)((sod % 3600L) / 60L);
    *s = (int)(sod % 60L);
}

/* ── Format helpers ──────────────────────────────────────────────────────── */

static void put2(char *buf, int *pos, int v) {
    buf[(*pos)++] = (char)('0' + v / 10);
    buf[(*pos)++] = (char)('0' + v % 10);
}

static void put4(char *buf, int *pos, int v) {
    buf[(*pos)++] = (char)('0' + v / 1000);
    buf[(*pos)++] = (char)('0' + (v / 100) % 10);
    buf[(*pos)++] = (char)('0' + (v / 10) % 10);
    buf[(*pos)++] = (char)('0' + v % 10);
}

static const char *MONTH_ABBR[13] = {
    "", "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};
static const char *MONTH_FULL[13] = {
    "", "January","February","March","April","May","June",
    "July","August","September","October","November","December"
};
static const char *DOW_ABBR[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
static const char *DOW_FULL[7] = {
    "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};

/* Tomohiko Sakamoto's algorithm — day of week 0=Sun */
static int day_of_week(int y, int m, int d) {
    static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    y -= (m < 3);
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

static void append_str(char *buf, int *pos, const char *s) {
    while (*s) buf[(*pos)++] = *s++;
}

/* ── Parse helpers ───────────────────────────────────────────────────────── */

static int parse_int(const char *s, int *pos, int digits) {
    int v = 0;
    for (int i = 0; i < digits && s[*pos] >= '0' && s[*pos] <= '9'; i++)
        v = v * 10 + (s[(*pos)++] - '0');
    return v;
}

static int64_t ymd_to_unix(int y, int m, int d) {
    /* Rata Die serial number, then subtract 1970-01-01 */
    if (m < 3) { y--; m += 12; }
    long era = (long)y / 400;
    long yoe = (long)y - era * 400;
    long doy = (153L * (long)(m - 3) + 2L) / 5L + (long)d - 1L;
    long doe = yoe * 365L + yoe / 4L - yoe / 100L + doy;
    long rd  = era * 146097L + doe - 719468L;
    return (int64_t)rd * 86400LL;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Public API                                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

void _F6fly_os7timeNow_Cfly_time(void *err_ctx, fly_time *out) {
    (void)err_ctx;
    linux_timespec ts;
    __os_sc2(SYS_clock_gettime, (long)CLOCK_REALTIME, (long)&ts);
    out->sec  = (int64_t)ts.tv_sec;
    out->nsec = (int64_t)ts.tv_nsec;
}

void _F6fly_os13timeMonotonic_Cfly_time(void *err_ctx, fly_time *out) {
    (void)err_ctx;
    linux_timespec ts;
    __os_sc2(SYS_clock_gettime, (long)CLOCK_MONOTONIC, (long)&ts);
    out->sec  = (int64_t)ts.tv_sec;
    out->nsec = (int64_t)ts.tv_nsec;
}

void _F6fly_os9timeSleep_Cfly_duration(void *err_ctx, const fly_duration *d) {
    (void)err_ctx;
    if (!d || d->nsec <= 0) return;
    linux_timespec ts;
    ts.tv_sec  = (long)(d->nsec / FLY_SECOND);
    ts.tv_nsec = (long)(d->nsec % FLY_SECOND);
    linux_timespec rem;
    __os_sc2(SYS_nanosleep, (long)&ts, (long)&rem);
}

void _F6fly_os9timeSince_Cfly_time_Cfly_duration(void *err_ctx, const fly_time *t, fly_duration *out) {
    (void)err_ctx;
    fly_time now;
    _F6fly_os7timeNow_Cfly_time((void*)0, &now);
    _F6fly_os8timeDiff_Cfly_time_Cfly_time_Cfly_duration((void*)0, t, &now, out);
}

void _F6fly_os8timeDiff_Cfly_time_Cfly_time_Cfly_duration(void *err_ctx, const fly_time *a, const fly_time *b, fly_duration *out) {
    (void)err_ctx;
    int64_t sec  = b->sec  - a->sec;
    int64_t nsec = b->nsec - a->nsec;
    out->nsec = sec * FLY_SECOND + nsec;
}

void _F6fly_os7timeAdd_Cfly_time_Cfly_duration_Cfly_time(void *err_ctx, const fly_time *t, const fly_duration *d, fly_time *out) {
    (void)err_ctx;
    int64_t total_nsec = t->nsec + d->nsec;
    out->sec  = t->sec + total_nsec / FLY_SECOND;
    out->nsec = total_nsec % FLY_SECOND;
    if (out->nsec < 0) { out->sec--; out->nsec += FLY_SECOND; }
}

void _F6fly_os11timeCompare_Cfly_time_Cfly_time_i(void *err_ctx, const fly_time *a, const fly_time *b, int *out) {
    (void)err_ctx;
    if (a->sec < b->sec)                              { *out = -1; return; }
    if (a->sec > b->sec)                              { *out =  1; return; }
    if (a->nsec < b->nsec)                            { *out = -1; return; }
    if (a->nsec > b->nsec)                            { *out =  1; return; }
    *out = 0;
}

void _F6fly_os8timeUnix_Cfly_time_l(void *err_ctx, const fly_time *t, int64_t *out) {
    (void)err_ctx;
    *out = t->sec;
}

void _F6fly_os12timeUnixNano_Cfly_time_l(void *err_ctx, const fly_time *t, int64_t *out) {
    (void)err_ctx;
    *out = t->sec * FLY_SECOND + t->nsec;
}

void _F6fly_os12timeFromUnix_l_Cfly_time(void *err_ctx, int64_t sec, fly_time *out) {
    (void)err_ctx;
    out->sec  = sec;
    out->nsec = 0;
}

void _F6fly_os16timeFromUnixNano_l_Cfly_time(void *err_ctx, int64_t nsec, fly_time *out) {
    (void)err_ctx;
    out->sec  = nsec / FLY_SECOND;
    out->nsec = nsec % FLY_SECOND;
    if (out->nsec < 0) { out->sec--; out->nsec += FLY_SECOND; }
}

/*
 * Go reference time: "Mon Jan 2 15:04:05 MST 2006"
 * Supported tokens: 2006, 01, 1, 02, 2, 15, 04, 4, 05, 5,
 *                   January, Jan, Monday, Mon
 */
void _F6fly_os10timeFormat_Cfly_time_Ss_Ss(void *err_ctx, const fly_time *t, const fly_string *pattern, fly_string *out) {
    (void)err_ctx;
    int year, month, day, hour, min, sec;
    unix_to_ymd(t->sec, &year, &month, &day);
    unix_to_hms(t->sec, &hour, &min, &sec);
    int dow = day_of_week(year, month, day);

    char buf[256];
    int pos = 0;
    int plen = pattern->size;
    const char *p = pattern->ptr;
    int i = 0;
    while (i < plen && pos < (int)sizeof(buf) - 10) {
        /* longest-first matching of Go reference tokens */
        if (i + 7 <= plen && p[i]=='M'&&p[i+1]=='o'&&p[i+2]=='n'&&p[i+3]=='d'&&
            p[i+4]=='a'&&p[i+5]=='y') { append_str(buf,&pos,DOW_FULL[dow]); i+=6; }
        else if (i+3<=plen&&p[i]=='M'&&p[i+1]=='o'&&p[i+2]=='n') {
            append_str(buf,&pos,DOW_ABBR[dow]); i+=3; }
        else if (i+7<=plen&&p[i]=='J'&&p[i+1]=='a'&&p[i+2]=='n'&&p[i+3]=='u') {
            append_str(buf,&pos,MONTH_FULL[month]); i+=7; /* "January" */ }
        else if (i+3<=plen&&p[i]=='J'&&p[i+1]=='a'&&p[i+2]=='n') {
            append_str(buf,&pos,MONTH_ABBR[month]); i+=3; }
        else if (i+4<=plen&&p[i]=='2'&&p[i+1]=='0'&&p[i+2]=='0'&&p[i+3]=='6') {
            put4(buf,&pos,year); i+=4; }
        else if (i+2<=plen&&p[i]=='0'&&p[i+1]=='1') { put2(buf,&pos,month); i+=2; }
        else if (i+1<=plen&&p[i]=='1'&&!(i>0&&p[i-1]=='0')) {
            buf[pos++]=(char)('0'+month/10); if(month>=10) buf[pos-1]='.'; /* fallthrough */
            /* simple: print month without leading zero */
            pos--; if(month>=10){buf[pos++]=(char)('0'+month/10);} buf[pos++]=(char)('0'+month%10); i++; }
        else if (i+2<=plen&&p[i]=='0'&&p[i+1]=='2') { put2(buf,&pos,day); i+=2; }
        else if (i+2<=plen&&p[i]=='1'&&p[i+1]=='5') { put2(buf,&pos,hour); i+=2; }
        else if (i+2<=plen&&p[i]=='0'&&p[i+1]=='4') { put2(buf,&pos,min); i+=2; }
        else if (i+2<=plen&&p[i]=='0'&&p[i+1]=='5') { put2(buf,&pos,sec); i+=2; }
        else { buf[pos++] = p[i++]; }
    }
    fstr_from_cstr(buf, pos, out);
}

void _F6fly_os9timeParse_Ss_Ss_Cfly_time(void *err_ctx, const fly_string *s, const fly_string *pattern, fly_time *out) {
    (void)err_ctx;
    out->sec  = 0;
    out->nsec = 0;
    if (!s || !pattern) return;
    char sbuf[128];
    int slen = s->size < 127 ? s->size : 127;
    for (int i = 0; i < slen; i++) sbuf[i] = s->ptr[i];
    sbuf[slen] = '\0';
    int year = 1970, month = 1, day = 1, hour = 0, min = 0, sec = 0;
    const char *p = pattern->ptr;
    int plen = pattern->size;
    int si = 0, pi = 0;
    while (pi < plen && si < slen) {
        if (pi+4<=plen&&p[pi]=='2'&&p[pi+1]=='0'&&p[pi+2]=='0'&&p[pi+3]=='6') {
            year = parse_int(sbuf, &si, 4); pi += 4; }
        else if (pi+2<=plen&&p[pi]=='0'&&p[pi+1]=='1') {
            month = parse_int(sbuf, &si, 2); pi += 2; }
        else if (pi+2<=plen&&p[pi]=='0'&&p[pi+1]=='2') {
            day = parse_int(sbuf, &si, 2); pi += 2; }
        else if (pi+2<=plen&&p[pi]=='1'&&p[pi+1]=='5') {
            hour = parse_int(sbuf, &si, 2); pi += 2; }
        else if (pi+2<=plen&&p[pi]=='0'&&p[pi+1]=='4') {
            min = parse_int(sbuf, &si, 2); pi += 2; }
        else if (pi+2<=plen&&p[pi]=='0'&&p[pi+1]=='5') {
            sec = parse_int(sbuf, &si, 2); pi += 2; }
        else { pi++; si++; } /* literal char: skip both */
    }
    out->sec  = ymd_to_unix(year, month, day) + (int64_t)hour*3600 + (int64_t)min*60 + sec;
    out->nsec = 0;
}

void _F6fly_os17timeDurationSecs_Cfly_duration_l(void *err_ctx, const fly_duration *d, int64_t *out) {
    (void)err_ctx;
    *out = d->nsec / FLY_SECOND;
}

void _F6fly_os19timeDurationMillis_Cfly_duration_l(void *err_ctx, const fly_duration *d, int64_t *out) {
    (void)err_ctx;
    *out = d->nsec / FLY_MILLISECOND;
}

void _F6fly_os19timeDurationMicros_Cfly_duration_l(void *err_ctx, const fly_duration *d, int64_t *out) {
    (void)err_ctx;
    *out = d->nsec / FLY_MICROSECOND;
}

void _F6fly_os18timeDurationFormat_Cfly_duration_Ss(void *err_ctx, const fly_duration *d, fly_string *out) {
    (void)err_ctx;
    if (!d || d->nsec == 0) { fstr_from_cstr("0s", 2, out); return; }
    char buf[64];
    int pos = 0;
    int64_t n = i64abs(d->nsec);
    if (d->nsec < 0) buf[pos++] = '-';
    if (n >= FLY_HOUR) {
        int64_t h = n / FLY_HOUR; n %= FLY_HOUR;
        int64_t m = n / FLY_MINUTE; n %= FLY_MINUTE;
        int64_t s = n / FLY_SECOND;
        /* print hours */
        if (h >= 100) buf[pos++] = (char)('0' + h/100);
        if (h >= 10)  buf[pos++] = (char)('0' + (h/10)%10);
        buf[pos++] = (char)('0' + h%10); buf[pos++] = 'h';
        buf[pos++] = (char)('0' + m/10); buf[pos++] = (char)('0' + m%10); buf[pos++] = 'm';
        buf[pos++] = (char)('0' + s/10); buf[pos++] = (char)('0' + s%10); buf[pos++] = 's';
    } else if (n >= FLY_MINUTE) {
        int64_t m = n / FLY_MINUTE; n %= FLY_MINUTE;
        int64_t s = n / FLY_SECOND;
        if (m >= 10) buf[pos++] = (char)('0' + m/10);
        buf[pos++] = (char)('0' + m%10); buf[pos++] = 'm';
        buf[pos++] = (char)('0' + s/10); buf[pos++] = (char)('0' + s%10); buf[pos++] = 's';
    } else if (n >= FLY_SECOND) {
        int64_t s = n / FLY_SECOND;
        int64_t ms = (n % FLY_SECOND) / FLY_MILLISECOND;
        if (s >= 10) buf[pos++] = (char)('0' + s/10);
        buf[pos++] = (char)('0' + s%10);
        if (ms > 0) {
            buf[pos++] = '.';
            buf[pos++] = (char)('0' + ms/100);
            buf[pos++] = (char)('0' + (ms/10)%10);
            buf[pos++] = (char)('0' + ms%10);
        }
        buf[pos++] = 's';
    } else if (n >= FLY_MILLISECOND) {
        int64_t ms = n / FLY_MILLISECOND;
        if (ms >= 100) buf[pos++] = (char)('0' + ms/100);
        if (ms >= 10)  buf[pos++] = (char)('0' + (ms/10)%10);
        buf[pos++] = (char)('0' + ms%10); buf[pos++] = 'm'; buf[pos++] = 's';
    } else if (n >= FLY_MICROSECOND) {
        int64_t us = n / FLY_MICROSECOND;
        if (us >= 100) buf[pos++] = (char)('0' + us/100);
        if (us >= 10)  buf[pos++] = (char)('0' + (us/10)%10);
        buf[pos++] = (char)('0' + us%10);
        /* µ in UTF-8: 0xC2 0xB5 */
        buf[pos++] = '\xc2'; buf[pos++] = '\xb5'; buf[pos++] = 's';
    } else {
        int64_t ns = n;
        if (ns >= 100) buf[pos++] = (char)('0' + ns/100);
        if (ns >= 10)  buf[pos++] = (char)('0' + (ns/10)%10);
        buf[pos++] = (char)('0' + ns%10); buf[pos++] = 'n'; buf[pos++] = 's';
    }
    fstr_from_cstr(buf, pos, out);
}
