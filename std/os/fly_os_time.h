/*===-- std/os/fly_os_time.h - fly.os time API (synchronous only) -------===
 *
 * Async timers and tickers are deferred to fly.sync.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_TIME_H
#define FLY_OS_TIME_H

#include "fly_os_io.h"

void fly_time_now           (fly_time *out);
void fly_time_monotonic     (fly_time *out);
void fly_time_sleep         (const fly_duration *d);
void fly_time_since         (const fly_time *t, fly_duration *out);
void fly_time_diff          (const fly_time *a, const fly_time *b, fly_duration *out);
void fly_time_add           (const fly_time *t, const fly_duration *d, fly_time *out);
void fly_time_compare       (const fly_time *a, const fly_time *b, int *out);
void fly_time_unix          (const fly_time *t, int64_t *out);
void fly_time_unixNano      (const fly_time *t, int64_t *out);
void fly_time_fromUnix      (int64_t sec, fly_time *out);
void fly_time_fromUnixNano  (int64_t nsec, fly_time *out);
void fly_time_format        (const fly_time *t, const fly_string *pattern, fly_string *out);
void fly_time_parse         (const fly_string *s, const fly_string *pattern, fly_time *out);
void fly_time_durationSecs  (const fly_duration *d, int64_t *out);
void fly_time_durationMillis(const fly_duration *d, int64_t *out);
void fly_time_durationMicros(const fly_duration *d, int64_t *out);
void fly_time_durationFormat(const fly_duration *d, fly_string *out);

#endif /* FLY_OS_TIME_H */
