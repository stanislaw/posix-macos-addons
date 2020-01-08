#include "time-addons.h"

#include "semaphore-mac.h"

#include <gtest/gtest.h>

#include <iostream>


TEST(Semaphore_Test, 01) {
  timespec tm;
  int msecs = 1;

  clock_gettime( CLOCK_REALTIME,  &tm );

  /* add the delay to the current time */
  tm.tv_sec  += (time_t) (msecs / 1000) ;
  /* convert residue ( msecs )  to nanoseconds */
  tm.tv_nsec +=  (msecs % 1000) * 1000000L ;

  if (tm.tv_nsec  >= 1000000000L) {
    tm.tv_nsec -= 1000000000L ;
    tm.tv_sec ++ ;
  }

  const char sem_name[] = "/test-sem";
  {
    sem_t *sptr = sem_open(sem_name, 0, 0644, 0);
    if (sptr != SEM_FAILED) {
      sem_close(sptr);
      sem_unlink(sem_name);
    }
  }

  sem_t *sptr = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 0);
  assert(sptr != SEM_FAILED);

  int result = sem_timedwait(sptr, &tm);
  ASSERT_EQ(result, -1);
  ASSERT_EQ(errno, ETIMEDOUT);
}
