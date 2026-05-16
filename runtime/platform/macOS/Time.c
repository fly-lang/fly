/*===-- runtime/macOS/Time.c - Time primitives via libSystem -------------===*/

#include "../Runtime.h"
#include "LibSystem.h"

i32 time_realtime(i64 *sec_out, i64 *nsec_out)
{
    macos_timespec_t ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    if (r != 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

i32 time_monotonic(i64 *sec_out, i64 *nsec_out)
{
    macos_timespec_t ts;
    int r = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (r != 0) { *sec_out = 0; *nsec_out = 0; return -1; }
    *sec_out  = (i64)ts.tv_sec;
    *nsec_out = (i64)ts.tv_nsec;
    return 0;
}

/* Single-value time helpers (Fly-compatible: one return value each) */
i64 time_now_ns(void)
{
    macos_timespec_t ts;
    return (clock_gettime(CLOCK_REALTIME, &ts) == 0)
        ? (i64)((long long)ts.tv_sec * 1000000000LL + ts.tv_nsec) : -1;
}

i64 time_mono_ns(void)
{
    macos_timespec_t ts;
    return (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        ? (i64)((long long)ts.tv_sec * 1000000000LL + ts.tv_nsec) : -1;
}

void time_sleep(i64 sec, i64 nsec)
{
    macos_timespec_t req, rem;
    req.tv_sec  = (long long)sec;
    req.tv_nsec = (long)nsec;
    nanosleep(&req, &rem);
}
