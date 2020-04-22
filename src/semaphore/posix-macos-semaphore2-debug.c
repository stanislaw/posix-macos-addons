#include "posix-macos-semaphore2-debug.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

static void timespec_diff(const struct timespec *lhs,
                          const struct timespec *rhs,
                          struct timespec *out) {
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

/// The code that does not balance calls to init, wait, post and destroy
/// results in
/// "BUG IN CLIENT OF LIBDISPATCH: Semaphore object deallocated while in use"
/// crashes. There are two cases:
/// 1) disposing a semaphore which is being waited on
/// 2) calling wait on a disposed semaphore
/// We prefer to keep a sync queue to count calls to init, wait, post, destroy
/// to prevent the code from crashing.
/// TODO: Would be great to remove this in the future.
static dispatch_queue_t sync_queue() {
  static dispatch_once_t guard;
  static dispatch_queue_t queue;
  dispatch_once(&guard, ^{
    queue =
      dispatch_queue_create("semaphore sync queue", DISPATCH_QUEUE_CONCURRENT);
  });
  return queue;
}

static uint64_t SEMAPHORE_ALREADY_RELEASED = UINT64_MAX - 0xdeadbeafU;

static void reset_count(mac_sem2_t *psem) {
  dispatch_queue_set_specific(sync_queue(), *psem, (void *)0, NULL);
}

static void increment_count(mac_sem2_t *psem) {
  uint64_t count = (uint64_t)dispatch_queue_get_specific(sync_queue(), *psem);
  assert(count != (SEMAPHORE_ALREADY_RELEASED - 1));
  assert(count != SEMAPHORE_ALREADY_RELEASED);
  count++;
  dispatch_queue_set_specific(sync_queue(), *psem, (void *)count, NULL);
}

static void decrement_count(mac_sem2_t *psem) {
  uint64_t count = (uint64_t)dispatch_queue_get_specific(sync_queue(), *psem);
  assert(count != SEMAPHORE_ALREADY_RELEASED);
  count = count == 0 ? 0 : count - 1;
  dispatch_queue_set_specific(sync_queue(), *psem, (void *)count, NULL);
}

static void finalize_count(mac_sem2_t *psem) {
  dispatch_queue_set_specific(
    sync_queue(), *psem, (void *)SEMAPHORE_ALREADY_RELEASED, NULL);
}

static uint64_t get_count(mac_sem2_t *psem) {
  return (uint64_t)dispatch_queue_get_specific(sync_queue(), *psem);
}

int mac_sem2_debug_init(mac_sem2_t *psem, int flags, unsigned count) {
  dispatch_sync(sync_queue(), ^{
    *psem = dispatch_semaphore_create(count);
    reset_count(psem);
  });

  return 0;
}

int mac_sem2_debug_destroy(mac_sem2_t *psem) {
  dispatch_barrier_sync(sync_queue(), ^{
    uint64_t count = get_count(psem);
    if (count == SEMAPHORE_ALREADY_RELEASED) {
      printf("warning: mac_sem2_debug_destroy: the over-release is detected\n");
      return;
    } else if (count > 0) {
      printf("warning: mac_sem2_debug_destroy: the unbalanced calls to wait/post is "
             "detected with count == %u\n",
             (uint32_t)count);
      for (int i = 0; i < count; i++) {
        dispatch_semaphore_signal(*psem);
      }
    }
    finalize_count(psem);
    dispatch_release(*psem);
  });
  return 0;
}

int mac_sem2_debug_post(mac_sem2_t *psem) {
  dispatch_sync(sync_queue(), ^{
    uint64_t count = get_count(psem);
    if (count == SEMAPHORE_ALREADY_RELEASED) {
      printf(
        "warning: mac_sem2_debug_post: post on a released semaphore is detected\n");
      return;
    }
    decrement_count(psem);
    dispatch_semaphore_signal(*psem);
  });
  return 0;
}

int mac_sem2_debug_trywait(mac_sem2_t *psem) {
  __block int result;
  dispatch_sync(sync_queue(), ^{
    uint64_t count = get_count(psem);
    if (count == SEMAPHORE_ALREADY_RELEASED) {
      printf("warning: mac_sem2_debug_trywait: wait on a released semaphore is "
             "detected\n");
      result = 0;
      return;
    }
    increment_count(psem);
    result = dispatch_semaphore_wait(*psem, DISPATCH_TIME_NOW);
    if (result != 0) {
      decrement_count(psem);
      errno = ETIMEDOUT;
      result = -1;
      return;
    }
    result = 0;
  });
  return result;
}

int mac_sem2_debug_wait(mac_sem2_t *psem) {
  __block int result;
  dispatch_sync(sync_queue(), ^{
    uint64_t count = get_count(psem);
    if (count == SEMAPHORE_ALREADY_RELEASED) {
      printf(
        "warning: mac_sem2_debug_wait: wait on a released semaphore is detected\n");
      result = 0;
      return;
    }
    increment_count(psem);
  });

  do {
    dispatch_sync(sync_queue(), ^{
      uint64_t count = get_count(psem);
      if (count == SEMAPHORE_ALREADY_RELEASED) {
        printf(
          "warning: mac_sem2_debug_wait: wait on a released semaphore is detected\n");
        result = 0;
        return;
      }
      result = dispatch_semaphore_wait(*psem, DISPATCH_TIME_NOW);
      if (result != 0) {
        decrement_count(psem);
        errno = ETIMEDOUT;
        result = -1;
        return;
      }
    });
  } while (result != 0);
  return result;
}

int mac_sem2_debug_timedwait(mac_sem2_t *psem, const struct timespec *abstim) {
  __block int result;
  dispatch_sync(sync_queue(), ^{
    uint64_t count = get_count(psem);
    if (count == SEMAPHORE_ALREADY_RELEASED) {
      printf("warning: mac_sem2_debug_timedwait: wait on a released semaphore is "
             "detected\n");
      result = 0;
      return;
    }
    increment_count(psem);

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    struct timespec diff;
    timespec_diff(&now, abstim, &diff);

    long diff_ns = diff.tv_sec * NSEC_PER_SEC + diff.tv_nsec;
    dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, diff_ns);

    result = dispatch_semaphore_wait(*psem, timeout);
    if (result != 0) {
      decrement_count(psem);
      errno = ETIMEDOUT;
      result = -1;
      return;
    }
    result = 0;
    return;
  });
  return result;
}

int mac_sem2_debug_getvalue(mac_sem2_t *sem, int *sval) {
  assert(0 && "Not implemented");
  return 0;
}
