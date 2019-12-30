#include <mqueue/mqueue.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

int main() {

  __unused mqd_t mqd = mq_open("hello", 0);


}
