#include "posix-mac-semaphore2.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

static
void timespec_diff(const struct timespec* lhs,
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

/// NASA CFS tests do not balance calls to init, wait, post and destroy and
/// this results in
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
    queue = dispatch_queue_create("semaphore sync queue", DISPATCH_QUEUE_SERIAL);
  });
  return queue;
}

static uint64_t MAGIC_NUMBER = UINT64_MAX - 0xdeadbeafU;

void reset_count(mac_sem2_t *psem) {
  dispatch_sync(sync_queue(), ^{
    dispatch_queue_set_specific(sync_queue(), *psem, (void *)0, NULL);
  });
}

void increment_count(mac_sem2_t *psem) {
  dispatch_sync(sync_queue(), ^{
    uint64_t
      count = (uint64_t) dispatch_queue_get_specific(sync_queue(), *psem);
    assert(count != (MAGIC_NUMBER - 1));
    assert(count != MAGIC_NUMBER);
    count++;
    dispatch_queue_set_specific(sync_queue(), *psem, (void *) count, NULL);
  });
}

void decrement_count(mac_sem2_t *psem) {
  dispatch_sync(sync_queue(), ^{
    uint64_t
      count = (uint64_t) dispatch_queue_get_specific(sync_queue(), *psem);
    assert(count != MAGIC_NUMBER);
    count = count == 0 ? 0 : count - 1;
    dispatch_queue_set_specific(sync_queue(), *psem, (void *) count, NULL);
  });
}

void finalize_count(mac_sem2_t *psem) {
  dispatch_sync(sync_queue(), ^{
    uint64_t count = MAGIC_NUMBER;
    dispatch_queue_set_specific(sync_queue(), *psem, (void *) count, NULL);
  });
}

uint64_t get_count(mac_sem2_t *psem) {
  __block uint64_t count;
  dispatch_sync(sync_queue(), ^{
    count = (uint64_t) dispatch_queue_get_specific(sync_queue(), *psem);
  });
  return count;
}

int mac_sem2_init(mac_sem2_t *psem, int flags, unsigned count) {
  *psem = dispatch_semaphore_create(count);
  reset_count(psem);
  return 0;
}

int mac_sem2_destroy(mac_sem2_t *psem) {
  uint64_t count = get_count(psem);
  if (count == MAGIC_NUMBER) {
    printf("warning: mac_sem2_destroy: the over-release is detected\n");
    return 0;
  } else if (count > 0) {
    printf("warning: mac_sem2_destroy: the unbalanced calls to wait/post is detected with count == %u\n", (uint32_t)count);
    for (int i = 0; i < count; i++) {
      dispatch_semaphore_signal(*psem);
    }
  }
  finalize_count(psem);
  dispatch_release(*psem);
  return 0;
}

int mac_sem2_post(mac_sem2_t *psem) {
  uint64_t count = get_count(psem);
  if (count == MAGIC_NUMBER) {
    printf("warning: mac_sem2_post: post on released semaphore is detected\n");
    return 0;
  }
  decrement_count(psem);
  dispatch_semaphore_signal(*psem);
  return 0;
}

int mac_sem2_trywait(mac_sem2_t *psem) {
  increment_count(psem);
  int result = dispatch_semaphore_wait(*psem, DISPATCH_TIME_NOW);
  if (result != 0) {
    decrement_count(psem);
    errno = ETIMEDOUT;
    return -1;
  }
  return 0;
}

int mac_sem2_wait(mac_sem2_t *psem) {
  uint64_t count = get_count(psem);
  if (count == MAGIC_NUMBER) {
    printf("warning: mac_sem2_wait: wait on released semaphore is detected\n");
    return 0;
  }
  increment_count(psem);
  dispatch_semaphore_wait(*psem, DISPATCH_TIME_FOREVER);
  return 0;
}

int mac_sem2_timedwait(mac_sem2_t *psem, const struct timespec *abstim) {
  increment_count(psem);

  struct timespec now;
  clock_gettime( CLOCK_REALTIME, &now);

  struct timespec diff;
  timespec_diff(&now, abstim, &diff);

  long diff_ns = diff.tv_sec * NSEC_PER_SEC + diff.tv_nsec;
  dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, diff_ns);

  int result = dispatch_semaphore_wait(*psem, timeout);
  if (result != 0) {
    decrement_count(psem);
    errno = ETIMEDOUT;
    return -1;
  }
  return 0;
}

int mac_sem2_getvalue(mac_sem2_t *sem, int *sval) {
  assert(0 && "Not implemented");
  return 0;
}
