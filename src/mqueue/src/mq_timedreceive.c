#include "mqueue.h"

#include <assert.h>

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr,
                        size_t msg_len, unsigned *msg_prio,
                        const struct timespec *abs_timeout) {
  assert(0 && "Should not reach here");
  return -1;
}
