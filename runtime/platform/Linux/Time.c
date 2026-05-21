/*===-- runtime/Linux/Time.c - Time primitives via clock_gettime/nanosleep ===*/

#include "../Runtime.h"
#include "Syscall.h"

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

typedef struct { long tv_sec; long tv_nsec; } linux_timespec_t;

i32 time_realtime(i64 *sec_out, i64 *nsec_out)
{
    linux_timespec_t ts;
    long r = __syscall2(SYS_clock_gettime, (long)CLOCK_REALTIME, (long)&ts);
    if (r < 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

i32 time_monotonic(i64 *sec_out, i64 *nsec_out)
{
    linux_timespec_t ts;
    long r = __syscall2(SYS_clock_gettime, (long)CLOCK_MONOTONIC, (long)&ts);
    if (r < 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

/* Single-value time helpers (Fly-compatible: one return value each) */
i64 time_now_ns(void)
{
    linux_timespec_t ts;
    long r = __syscall2(SYS_clock_gettime, (long)CLOCK_REALTIME, (long)&ts);
    return (r < 0) ? -1 : ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

i64 time_mono_ns(void)
{
    linux_timespec_t ts;
    long r = __syscall2(SYS_clock_gettime, (long)CLOCK_MONOTONIC, (long)&ts);
    return (r < 0) ? -1 : ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void time_sleep(i64 sec, i64 nsec)
{
    linux_timespec_t req, rem;
    req.tv_sec  = (long)sec;
    req.tv_nsec = (long)nsec;
    __syscall2(SYS_nanosleep, (long)&req, (long)&rem);
}
