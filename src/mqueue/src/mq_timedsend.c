#include "mqueue.h"

#include <assert.h>

int mq_timedsend(mqd_t mqdes, const char *msg_ptr,
                 size_t msg_len, unsigned msg_prio,
                 const struct timespec *abs_timeout) {
  assert(0 && "Should not reach here");
  return -1;
}