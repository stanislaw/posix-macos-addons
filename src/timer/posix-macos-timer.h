#pragma once

#include <signal.h>
#include <time.h>
#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t timer_t;

struct itimerspec {
  struct timespec it_interval;  /* Timer interval */
  struct timespec it_value;     /* Initial expiration */
};

int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);

int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value);

int timer_delete(timer_t timerid);

int timer_poll(timer_t timerid);

#ifdef __cplusplus
}
#endif
