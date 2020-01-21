#include	"mqueue.h"

int mq_unlink(const char *pathname) {
  if (unlink(pathname) == -1) {
    return (-1);
  }
  return(0);
}

void
Mymq_unlink(const char *pathname) {
  if (mq_unlink(pathname) == -1) {
    err_sys("mymq_unlink error");
  }
}
