#include "posix-macos-timer.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(TimerTest, 01) {
  struct timespec now;

  assert(clock_gettime(CLOCK_REALTIME, &now) == 0);

  struct timespec deadline;
  clock_gettime(CLOCK_MONOTONIC, &deadline);

  struct itimerspec its;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 1;
  its.it_interval.tv_nsec = 0;

  struct sigevent dummy_sev;

  timer_t timerid;

  if (timer_create(CLOCK_REALTIME, &dummy_sev, &timerid) == -1) {
    perror("timer_create");
    exit(1);
  }

  if (timer_settime(timerid, 0, &its, NULL) == -1) {
    perror("timer_settime");
    exit(1);
  }

  int counter = 0;
  while (counter != 10) {
    timer_poll(timerid);
    printf("tick: %d\n", counter++);
  };
}
