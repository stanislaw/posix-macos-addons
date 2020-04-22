#include "posix-macos-time.h"

#include "posix-macos-semaphore.h"

#include <gtest/gtest.h>

#include <iostream>


TEST(Semaphore_Test, 01) {
  timespec tm;
  int msecs = 1000;

  clock_gettime( CLOCK_REALTIME,  &tm );

  /* add the delay to the current time */
  tm.tv_sec  += (time_t) (msecs / 1000) ;
  /* convert residue ( msecs )  to nanoseconds */
  tm.tv_nsec +=  (msecs % 1000) * 1000000L ;

  if (tm.tv_nsec  >= 1000000000L) {
    tm.tv_nsec -= 1000000000L ;
    tm.tv_sec ++ ;
  }

  mac_sem_t sem;
  assert(mac_sem_init(&sem, 0, 0) == 0);

  int result = mac_sem_timedwait(&sem, &tm);
  ASSERT_EQ(result, -1);
  ASSERT_EQ(errno, ETIMEDOUT);
}
