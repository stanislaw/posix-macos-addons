#include "mqueue.h"

#include "posix-macos-time.h"

#include <gtest/gtest.h>

#include <iostream>

#define BUFFLEN 100
static char msg[BUFFLEN]; /* receive buffer */

//static char msg0[] = "";
static char msg1[] = "1";
static char msg2[] = "22";
static char msg3[] = "333";
//static char msg4[] = "4444";
//static char msg5[] = "55555";
//static char msg6[] = "666666";
//static char msg7[] = "7777777";
//static char msg8[] = "88888888";

static const char message_queue_name[] = "/test_mqueue";

class MessageQueue_TimedSendReceive_Test : public ::testing::Test {
protected:
  void SetUp() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }

  void TearDown() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }

  int rc;
  mqd_t mqd;
  unsigned int prio;
  struct sigevent sigev;
  struct mq_attr info;
};

TEST_F(MessageQueue_TimedSendReceive_Test, timedSend) {
  struct mq_attr attr;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = 7;

  mqd = mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  assert(mq_send(mqd, msg1, 1, 1) == 0);

  struct timespec wait_time;
  assert(clock_gettime(CLOCK_REALTIME, &wait_time) == 0);
  wait_time.tv_sec += 2;

  int result = mq_timedsend(mqd, msg2, 2, 2, &wait_time);
  ASSERT_EQ(errno, ETIMEDOUT);
  ASSERT_EQ(result, -1);
}

TEST_F(MessageQueue_TimedSendReceive_Test, timedReceive_messageExists) {
  struct mq_attr attr;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = 7;

  mqd = mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  assert(mq_send(mqd, msg3, 3, 3) == 0);
  /// assert(errno == 0);

  struct timespec wait_time;
  assert(clock_gettime(CLOCK_REALTIME, &wait_time) == 0);
  wait_time.tv_sec += 1;
  unsigned prio;

  ssize_t received = mq_timedreceive(mqd, msg, BUFFLEN, &prio, &wait_time);
  ASSERT_EQ(received, 3);
}

TEST_F(MessageQueue_TimedSendReceive_Test, timedReceive_withNoMessages) {
  struct mq_attr attr;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = 7;

  mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  struct timespec wait_time;
  assert(clock_gettime(CLOCK_REALTIME, &wait_time) == 0);
  wait_time.tv_sec += 1;

  unsigned prio;
  ssize_t received = mq_timedreceive(mqd, msg, BUFFLEN, &prio, &wait_time);
  ASSERT_EQ(errno, ETIMEDOUT);
  ASSERT_EQ(received, -1);

  assert(mq_send(mqd, msg1, 1, 1) == 0);

  assert(clock_gettime(CLOCK_REALTIME, &wait_time) == 0);
  wait_time.tv_sec += 2;
  received = mq_timedreceive(mqd, msg, BUFFLEN, &prio, &wait_time);
  ASSERT_EQ(received, 1);
}
