#include "posix-macos-timer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define TIMERS_MAX 100U

typedef struct {
  dispatch_source_t source;
  dispatch_queue_t queue;
  dispatch_semaphore_t semaphore;
} timer_entry;

typedef struct timer_storage_t {
  timer_entry timers[TIMERS_MAX];
  uint32_t count;
} timer_storage_t;
static timer_storage_t timer_storage = { .timers = {{0}}, .count = 0 };

static dispatch_queue_t sync_queue() {
  static dispatch_once_t guard;
  static dispatch_queue_t queue;
  dispatch_once(&guard, ^{
    queue = dispatch_queue_create("timer sync queue", 0);
  });
  return queue;
}

static long timespec_to_ns(const struct timespec *value) {
  return value->tv_sec * NSEC_PER_SEC + value->tv_nsec;
}

int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid) {
  assert(timerid != NULL);

  __block timer_t new_timer_id;
  dispatch_sync(sync_queue(), ^{
    new_timer_id = timer_storage.count;
    timer_storage.count++;
  });

  timer_entry *entry = &timer_storage.timers[new_timer_id];

  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

  dispatch_queue_t queue = dispatch_queue_create("timerQueue", 0);

  dispatch_source_t new_timer =
    dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);

  dispatch_source_set_event_handler(new_timer, ^{
    dispatch_semaphore_signal(semaphore);
  });

  dispatch_source_set_cancel_handler(new_timer, ^{
    /// BUG IN CLIENT OF LIBDISPATCH: Semaphore object deallocated while in use
    /// This should never happen but it does in CFS OSAL tests for POSIX.
    /// Putting this additional dispatch_semaphore_signal for now because it
    /// makes the tests pass.
    /// https://stackoverflow.com/questions/8287621/why-does-this-code-cause-exc-bad-instruction
    dispatch_semaphore_signal(semaphore);

    dispatch_release(new_timer);
    dispatch_release(queue);
    dispatch_release(semaphore);
  });

  entry->queue = queue;
  entry->source = new_timer;
  entry->semaphore = semaphore;

  *timerid = new_timer_id;

  return 0;
}

int timer_settime(timer_t timerid,
                  int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value) {
  assert(timerid < TIMERS_MAX);
  assert(flags == 0 && "No flags are supported");

  timer_entry *timer = &timer_storage.timers[timerid];

  int64_t value_ns = timespec_to_ns(&new_value->it_value);
  int64_t interval_ns = timespec_to_ns(&new_value->it_interval);

  dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, value_ns);
  dispatch_source_set_timer(timer->source, start, interval_ns, 0);

  dispatch_resume(timer->source);

  return 0;
}

int timer_delete(timer_t timerid) {
  timer_entry *timer = &timer_storage.timers[timerid];
  dispatch_source_cancel(timer->source);
  return 0;
}

int timer_poll(timer_t timerid) {
  assert(timerid < TIMERS_MAX);

  timer_entry *timer = &timer_storage.timers[timerid];
  dispatch_semaphore_wait(timer->semaphore, DISPATCH_TIME_FOREVER);

  return 0;
}
