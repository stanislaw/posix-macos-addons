#include "posix-macos-time.h"

#include <gtest/gtest.h>

#include <iostream>

extern "C" {
int _clock_gettime_impl(clockid_t clk_id, struct timespec *tp);
}

TEST(NanoSleepTest, 01_monotonic_clock) {
  struct timespec now, then;

  assert(_clock_gettime_impl(CLOCK_MONOTONIC, &now) == 0);

  struct timespec deadline;
  clock_gettime(CLOCK_MONOTONIC, &deadline);

  deadline.tv_sec += 1;
  // // Normalize the time to account for the second boundary
  // if(deadline.tv_nsec >= 1000000000) {
  //   deadline.tv_nsec -= 1000000000;
  //   deadline.tv_sec++;
  // }

  int status = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
  ASSERT_EQ(status, 0);

  assert(_clock_gettime_impl(CLOCK_MONOTONIC, &then) == 0);
  auto diff = then.tv_sec - now.tv_sec;
  ASSERT_EQ(diff, 1);
}

TEST(NanoSleepTest, 02_realtime_clock) {
  struct timespec now, then;

  struct timespec deadline;
  deadline.tv_sec = 1;
  deadline.tv_nsec = 0;

  assert(_clock_gettime_impl(CLOCK_REALTIME, &now) == 0);

  int status = clock_nanosleep(CLOCK_REALTIME, 0, &deadline, NULL);
  ASSERT_EQ(status, 0);

  assert(_clock_gettime_impl(CLOCK_REALTIME, &then) == 0);

  auto diff = then.tv_sec - now.tv_sec;
  ASSERT_EQ(diff, 1);
}
