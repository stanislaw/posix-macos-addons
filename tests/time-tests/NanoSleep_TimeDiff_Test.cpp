#include "posix-mac-time.h"

#include <gtest/gtest.h>

#include <iostream>

static const int ONE_SECOND_NANOSECONDS = 1000000000;

TEST(NanoSleep_TimeDiff_Test, 01) {
  struct timespec lhs, rhs, result;

  assert(clock_gettime(CLOCK_REALTIME, &lhs) == 0);
  rhs = lhs;
  ++rhs.tv_sec;

  __timespec_diff(&lhs, &rhs, &result);

  ASSERT_EQ(result.tv_sec, 1);
  ASSERT_EQ(result.tv_nsec, 0);
}

TEST(NanoSleep_TimeDiff_Test, 02) {
  struct timespec lhs, rhs, result;

  assert(clock_gettime(CLOCK_REALTIME, &lhs) == 0);
  rhs = lhs;
  ++rhs.tv_nsec;

  __timespec_diff(&lhs, &rhs, &result);

  ASSERT_EQ(result.tv_sec, 0);
  ASSERT_EQ(result.tv_nsec, 1);
}

TEST(NanoSleep_TimeDiff_Test, 03) {
  struct timespec lhs, rhs, result;

  const long some_delta = 400;

  lhs.tv_sec = 1;
  lhs.tv_nsec = ONE_SECOND_NANOSECONDS - some_delta;

  rhs.tv_sec = 2;
  rhs.tv_nsec = 400;

  __timespec_diff(&lhs, &rhs, &result);

  ASSERT_EQ(result.tv_sec, 0);
  ASSERT_EQ(result.tv_nsec, some_delta * 2);
}
