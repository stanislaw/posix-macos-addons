#include "mac_timer.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(TimerTest, 01) {
  struct timespec now, then;

  assert(clock_gettime(CLOCK_REALTIME, &now) == 0);

  struct timespec deadline;
  clock_gettime(CLOCK_MONOTONIC, &deadline);

  struct itimerspec its;
  long long freq_nanosecs = NSEC_PER_SEC / 10;
  its.it_value.tv_sec = freq_nanosecs / 1000000000;
  its.it_value.tv_nsec = freq_nanosecs % 1000000000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  struct sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGKILL;
  sev.sigev_value.sival_ptr = NULL;

  timer_t timerid;

  if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
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
