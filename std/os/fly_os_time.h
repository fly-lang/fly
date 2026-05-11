/*===-- std/os/fly_os_time.h - fly.os time API (synchronous only) -------===
 *
 * Async timers and tickers are deferred to fly.sync.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_TIME_H
#define FLY_OS_TIME_H

#include "fly_os_io.h"

void _F6fly_os7timeNow_Cfly_time           (void *err_ctx, fly_time *out);
void _F6fly_os13timeMonotonic_Cfly_time    (void *err_ctx, fly_time *out);
void _F6fly_os9timeSleep_Cfly_duration     (void *err_ctx, const fly_duration *d);
void _F6fly_os9timeSince_Cfly_time_Cfly_duration (void *err_ctx, const fly_time *t, fly_duration *out);
void _F6fly_os8timeDiff_Cfly_time_Cfly_time_Cfly_duration (void *err_ctx, const fly_time *a, const fly_time *b, fly_duration *out);
void _F6fly_os7timeAdd_Cfly_time_Cfly_duration_Cfly_time  (void *err_ctx, const fly_time *t, const fly_duration *d, fly_time *out);
void _F6fly_os11timeCompare_Cfly_time_Cfly_time_i          (void *err_ctx, const fly_time *a, const fly_time *b, int *out);
void _F6fly_os8timeUnix_Cfly_time_l       (void *err_ctx, const fly_time *t, int64_t *out);
void _F6fly_os12timeUnixNano_Cfly_time_l  (void *err_ctx, const fly_time *t, int64_t *out);
void _F6fly_os12timeFromUnix_l_Cfly_time  (void *err_ctx, int64_t sec, fly_time *out);
void _F6fly_os16timeFromUnixNano_l_Cfly_time (void *err_ctx, int64_t nsec, fly_time *out);
void _F6fly_os10timeFormat_Cfly_time_Ss_Ss  (void *err_ctx, const fly_time *t, const fly_string *pattern, fly_string *out);
void _F6fly_os9timeParse_Ss_Ss_Cfly_time    (void *err_ctx, const fly_string *s, const fly_string *pattern, fly_time *out);
void _F6fly_os17timeDurationSecs_Cfly_duration_l   (void *err_ctx, const fly_duration *d, int64_t *out);
void _F6fly_os19timeDurationMillis_Cfly_duration_l (void *err_ctx, const fly_duration *d, int64_t *out);
void _F6fly_os19timeDurationMicros_Cfly_duration_l (void *err_ctx, const fly_duration *d, int64_t *out);
void _F6fly_os18timeDurationFormat_Cfly_duration_Ss (void *err_ctx, const fly_duration *d, fly_string *out);

#endif /* FLY_OS_TIME_H */
