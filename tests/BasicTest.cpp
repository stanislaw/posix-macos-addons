#include "mqueue.h"
#include "unpipc.h"

#include <gtest/gtest.h>

#include <iostream>

#define BUFFLEN 100
char msg[BUFFLEN]; /* receive buffer */

char msg0[] = "";
char msg1[] = "1";
char msg2[] = "22";
char msg3[] = "333";
char msg4[] = "4444";
char msg5[] = "55555";
char msg6[] = "666666";
char msg7[] = "7777777";
char msg8[] = "88888888";

Sigfunc *Signal(int signo, Sigfunc *func) /* for our signal() function */
{
  Sigfunc *sigfunc;

  if ((sigfunc = signal(signo, func)) == SIG_ERR) {
    assert(0 && "Should not reach here");
    err_sys("signal error");
  }

  return (sigfunc);
}

static const char message_queue_name[] = "test_mqueue";

class QueueTest : public ::testing::Test {
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

TEST_F(QueueTest, 01_gettingDefaultAttributes) {
  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);

  Mymq_getattr(mqd, &info);

  ASSERT_EQ(info.mq_maxmsg, 128);
  ASSERT_EQ(info.mq_msgsize, 1024);
  ASSERT_EQ(info.mq_flags, 0);
  ASSERT_EQ(info.mq_curmsgs, 0);

  Mymq_close(mqd);
}

TEST_F(QueueTest, 02_when_opening_closing_reopening_it_must_exist) {
  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);

  int result = mq_close(mqd);
  ASSERT_EQ(result, 0);

  mqd = mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);
  ASSERT_EQ(errno, EEXIST);
}

TEST_F(QueueTest, 03_creating_with_attributes) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_getattr(mqd, &info);

  ASSERT_EQ(info.mq_maxmsg, 4);
  ASSERT_EQ(info.mq_msgsize, 7);

  int result = mq_close(mqd);
  ASSERT_EQ(result, 0);
}

TEST_F(QueueTest, 04_sending_message_too_long) {
  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);

  Mymq_send(mqd, msg1, 1, 1);
  ssize_t rc = mq_receive(mqd, msg, BUFFLEN, &prio);

  /// TODO: Why is this? Compare to 05_sending below...
  ASSERT_EQ(rc, -1);
  ASSERT_TRUE(strcmp(strerror(errno), "Message too long") == 0);
}

TEST_F(QueueTest, 05_sending) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg1, 1, 1);
  ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);

  ASSERT_EQ(rc, 1);
  ASSERT_EQ(prio, 1);
}

TEST_F(QueueTest, 06_sending_0bytes) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg1, 0, 0);
  ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);

  ASSERT_EQ(rc, 0);
  ASSERT_EQ(prio, 0);
}

TEST_F(QueueTest, 07_sending_two_receiving_in_reverse_order) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg2, 2, 2);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 2);
    ASSERT_EQ(prio, 2);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 1);
    ASSERT_EQ(prio, 1);
  }
}

TEST_F(QueueTest, 08_sending_three_receiving_in_reverse_order) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg3, 3, 3);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 3);
    ASSERT_EQ(prio, 3);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 2);
    ASSERT_EQ(prio, 2);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 1);
    ASSERT_EQ(prio, 1);
  }
}

TEST_F(QueueTest, 09_sending_four_receiving_in_reverse_order) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg3, 3, 3);
  Mymq_send(mqd, msg3, 4, 4);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 4);
    ASSERT_EQ(prio, 4);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 3);
    ASSERT_EQ(prio, 3);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 2);
    ASSERT_EQ(prio, 2);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 1);
    ASSERT_EQ(prio, 1);
  }
}

TEST_F(QueueTest, 10_send_two_receive_in_order) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg3, 4, 4);
  Mymq_send(mqd, msg3, 3, 3);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg1, 1, 1);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 4);
    ASSERT_EQ(prio, 4);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 3);
    ASSERT_EQ(prio, 3);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 2);
    ASSERT_EQ(prio, 2);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 1);
    ASSERT_EQ(prio, 1);
  }
}

TEST_F(QueueTest, 11_writing_and_closing_reopening_and_reading) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg3, 4, 4);
  Mymq_send(mqd, msg3, 3, 3);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg1, 1, 1);

  Mymq_close(mqd);

  mqd = Mymq_open(message_queue_name, O_RDWR | O_NONBLOCK, FILE_MODE, &attr);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 4);
    ASSERT_EQ(prio, 4);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 3);
    ASSERT_EQ(prio, 3);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 2);
    ASSERT_EQ(prio, 2);
  }

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 1);
    ASSERT_EQ(prio, 1);
  }
}

TEST_F(QueueTest, 12_non_blocking_read) {
  struct mq_attr attr;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = 7;

  mqd =
    Mymq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

  Mymq_send(mqd, msg3, 4, 4);
  Mymq_close(mqd);

  mqd = Mymq_open(message_queue_name, O_RDWR | O_NONBLOCK, FILE_MODE, &attr);

  {
    ssize_t rc = Mymq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, 4);
    ASSERT_EQ(prio, 4);
  }

  /// check non-blocking receive
  {
    ssize_t rc = mq_receive(mqd, msg, BUFFLEN, &prio);
    ASSERT_EQ(rc, -1);
    ASSERT_EQ(errno, EAGAIN);
  }
}

TEST_F(QueueTest, 13_non_blocking_send) {
  struct mq_attr attr;
  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;

  mqd = Mymq_open(message_queue_name,
                  O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
                  FILE_MODE,
                  &attr);

  Mymq_send(mqd, msg3, 4, 4);
  Mymq_send(mqd, msg3, 3, 3);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg1, 1, 1);

  rc = mq_send(mqd, msg5, 5, 5);
  ASSERT_EQ(rc, -1);
  ASSERT_EQ(errno, EAGAIN);
}
