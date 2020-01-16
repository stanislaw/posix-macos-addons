#pragma once

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const int TIMER_ABSTIME;

void __timespec_diff(const struct timespec* lhs,
                     const struct timespec* rhs,
                     struct timespec* out);

int clock_nanosleep(clockid_t clock_id, int flags,
                    const struct timespec *request,
                    struct timespec *remain);

#ifdef __cplusplus
}
#endif
