#include "mqueue.h"

#include <stdio.h>

int mq_unlink(const char *pathname) {
  if (unlink(pathname) == -1) {
    return (-1);
  }
  return(0);
}

