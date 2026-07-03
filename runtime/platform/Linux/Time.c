/*===-- runtime/Linux/Time.c - Time primitives via libc clock_gettime ===*/

#include "../Runtime.h"

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

typedef struct { long tv_sec; long tv_nsec; } linux_timespec_t;

/* libc (forward-declared under -nostdinc). */
extern i32 clock_gettime(i32 clk_id, linux_timespec_t *tp);
extern i32 nanosleep(const linux_timespec_t *req, linux_timespec_t *rem);

i32 time_realtime(i64 *sec_out, i64 *nsec_out)
{
    linux_timespec_t ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

i32 time_monotonic(i64 *sec_out, i64 *nsec_out)
{
    linux_timespec_t ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

/* Single-value time helpers (Fly-compatible: one return value each) */
i64 time_now_ns(void)
{
    linux_timespec_t ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return -1;
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

i64 time_mono_ns(void)
{
    linux_timespec_t ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return -1;
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void time_sleep(i64 sec, i64 nsec)
{
    linux_timespec_t req, rem;
    req.tv_sec  = (long)sec;
    req.tv_nsec = (long)nsec;
    nanosleep(&req, &rem);
}
