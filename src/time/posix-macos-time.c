#include "posix-macos-time.h"

#include <assert.h>
#include <errno.h>


#include <stdatomic.h>
#include <sys/sysctl.h>

#include <mach/mach_time.h>
#include <mach/clock_types.h>

// _clock_gettime_impl, _boottime_fallback_usec and _mach_boottime_usec
// are based on https://opensource.apple.com/source/Libc/Libc-1439.141.1/gen/clock_gettime.c.auto.html
static uint64_t
_boottime_fallback_usec(void)
{
    struct timeval tv;
    size_t len = sizeof(tv);
    int ret = sysctlbyname("kern.boottime", &tv, &len, NULL, 0);
    if (ret == -1) return 0;
    return (uint64_t)tv.tv_sec * USEC_PER_SEC + (uint64_t)tv.tv_usec;
}

static int
_mach_boottime_usec(uint64_t *boottime, struct timeval *realtime)
{
    uint64_t bt1 = 0, bt2 = 0;
    int ret;
    do {
        bt1 = _boottime_fallback_usec();

        atomic_thread_fence(memory_order_seq_cst);

        ret = gettimeofday(realtime, NULL);
        if (ret != 0) return ret;

        atomic_thread_fence(memory_order_seq_cst);

        bt2 = _boottime_fallback_usec();
    } while (bt1 != bt2);
    *boottime = bt1;
    return 0;
}


int _clock_gettime_impl(clockid_t clk_id, struct timespec *tp)
{
    switch(clk_id){
    case CLOCK_REALTIME: {
        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        TIMEVAL_TO_TIMESPEC(&tv, tp);
        return ret;
    }
    case CLOCK_MONOTONIC: {
        struct timeval tv;
        uint64_t boottime_usec;
        int ret = _mach_boottime_usec(&boottime_usec, &tv);
        struct timeval boottime = {
            .tv_sec = boottime_usec / USEC_PER_SEC,
            .tv_usec = boottime_usec % USEC_PER_SEC
        };
        timersub(&tv, &boottime, &tv);
        TIMEVAL_TO_TIMESPEC(&tv, tp);
        return ret;
    }
    default:
        errno = EINVAL;
        return -1;
    }
}

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101200
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  return _clock_gettime_impl(clk_id, tp);
}

#endif

void __timespec_diff(const struct timespec* lhs,
                     const struct timespec* rhs,
                     struct timespec* out) {
  assert(lhs->tv_sec <= rhs->tv_sec);

  out->tv_sec = rhs->tv_sec - lhs->tv_sec;
  out->tv_nsec = rhs->tv_nsec - lhs->tv_nsec;

  if (out->tv_sec < 0) {
    out->tv_sec = 0;
    out->tv_nsec = 0;
    return;
  }

  if (out->tv_nsec < 0) {
    if (out->tv_sec == 0) {
      out->tv_sec = 0;
      out->tv_nsec = 0;
      return;
    }

    out->tv_sec = out->tv_sec - 1;
    out->tv_nsec = out->tv_nsec + NSEC_PER_SEC;
  }
}

int clock_nanosleep(clockid_t clock_id,
                    int flags,
                    const struct timespec *req,
                    struct timespec *rem) {

  assert(clock_id == CLOCK_REALTIME || clock_id == CLOCK_MONOTONIC);
  assert(0 <= req->tv_nsec && req->tv_nsec <= NSEC_PER_SEC);
  assert(flags == 0 || flags == TIMER_ABSTIME);
  assert(flags != TIMER_ABSTIME || clock_id == CLOCK_MONOTONIC);

  if (flags == TIMER_ABSTIME) {
    struct timespec now;
    struct timespec diff;

    if (clock_gettime(clock_id, &now) != 0) {
      return errno;
    }

    __timespec_diff(&now, req, &diff);

    return nanosleep(&diff, rem);
  }

  if (nanosleep(req, rem) != 0) {
    return errno;
  }

  return 0;
}
