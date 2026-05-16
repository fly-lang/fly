/*===-- runtime/Windows/Time.c - Time primitives via Win32 ---------------===*/

#include "../Runtime.h"
#include "Win32.h"

/* Windows FILETIME epoch: January 1, 1601, in 100-ns ticks.
 * Offset to Unix epoch (January 1, 1970): 116444736000000000 ticks. */
#define WIN_EPOCH_OFFSET 116444736000000000ULL

i32 time_realtime(i64 *sec_out, i64 *nsec_out)
{
    FILETIME_u ft;
    GetSystemTimeAsFileTime(&ft.ft);
    unsigned long long ticks = ft.val - WIN_EPOCH_OFFSET;
    *sec_out  = (i64)(ticks / 10000000ULL);
    *nsec_out = (i64)((ticks % 10000000ULL) * 100ULL);
    return 0;
}

i32 time_monotonic(i64 *sec_out, i64 *nsec_out)
{
    LARGE_INTEGER_rt cnt, freq;
    if (!QueryPerformanceCounter(&cnt) || !QueryPerformanceFrequency(&freq)
        || freq.QuadPart == 0) {
        *sec_out = 0; *nsec_out = 0; return -1;
    }
    long long ticks = cnt.QuadPart;
    long long hz    = freq.QuadPart;
    *sec_out  = (i64)(ticks / hz);
    *nsec_out = (i64)((ticks % hz) * 1000000000LL / hz);
    return 0;
}

/* Single-value time helpers (Fly-compatible: one return value each) */
i64 time_now_ns(void)
{
    FILETIME_u ft;
    GetSystemTimeAsFileTime(&ft.ft);
    unsigned long long ticks = ft.val - WIN_EPOCH_OFFSET;
    return (i64)(ticks * 100ULL);
}

i64 time_mono_ns(void)
{
    LARGE_INTEGER_rt cnt, freq;
    if (!QueryPerformanceCounter(&cnt) || !QueryPerformanceFrequency(&freq)
        || freq.QuadPart == 0) return -1;
    return (i64)(cnt.QuadPart * 1000000000LL / freq.QuadPart);
}

void time_sleep(i64 sec, i64 nsec)
{
    /* Convert to milliseconds, rounding up */
    long long ms = sec * 1000LL + nsec / 1000000LL;
    if (ms < 0) ms = 0;
    Sleep((DWORD)ms);
}
