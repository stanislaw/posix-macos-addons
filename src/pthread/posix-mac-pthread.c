#include "posix-mac-pthread.h"

int pthread_setschedprio(pthread_t thread, int prio) {
  /// TODO/Stanislaw: read the existing parameters from threads, not create new.
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  struct sched_param parm;
  parm.sched_priority = prio;
  int result = pthread_attr_setschedparam(&attr, &parm);

  return result;
}
