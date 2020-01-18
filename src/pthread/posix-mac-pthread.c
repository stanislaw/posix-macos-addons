#include "posix-mac-pthread.h"

int pthread_setschedprio(pthread_t thread, int prio) {
  struct sched_param param;
  int current_policy;

  int result = pthread_getschedparam(thread, &current_policy, &param);
  if (result != 0) {
    return result;
  }

  param.sched_priority = prio;
  result = pthread_setschedparam(thread, current_policy, &param);

  return result;
}
