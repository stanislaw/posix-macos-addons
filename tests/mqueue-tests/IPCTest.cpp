#include "mqueue.h"

#include <gtest/gtest.h>

#include <iostream>

#define BUFFLEN 100
static char msg[BUFFLEN]; /* receive buffer */

//static char msg0[] = "";
static char msg1[] = "1";
//static char msg2[] = "22";
static char msg3[] = "333";
//static char msg4[] = "4444";
static char msg5[] = "55555";
//static char msg6[] = "666666";
//static char msg7[] = "7777777";
//static char msg8[] = "88888888";

static const char message_queue_name[] = "test_mqueue";

static const int CUSTOM_SUCCESS_EXIT_CODE = 47;

class IPCTest : public ::testing::Test {
protected:
  void SetUp() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }

  void TearDown() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }
};

TEST_F(IPCTest, 01_parent_sends_to_child) {
  /* make certain parent can send messages to child, and vice versa */
  struct mq_attr attr;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 7;

  mqd_t mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  pid_t child_pid = fork();
  if (child_pid == -1) {
    printf("fork error\n");
    assert(0);
  }

  if (child_pid == 0) {
    sleep(2); /* let parent send two messages */

    unsigned int prio;
    int rc = Mymq_receive(mqd, msg, attr.mq_msgsize, &prio);
    assert(rc == 3);
    assert(prio == 3);

    rc = Mymq_receive(mqd, msg, attr.mq_msgsize, &prio);
    assert(rc == 1);
    assert(prio == 1);

    exit(CUSTOM_SUCCESS_EXIT_CODE);
  }

  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg3, 3, 3);

  int exit_status_raw;
  while (waitpid(child_pid, &exit_status_raw, 0) == -1) {
    if (errno == EINTR) {
      printf("EINTR\n");
    }
  }
  ASSERT_EQ(WEXITSTATUS(exit_status_raw), CUSTOM_SUCCESS_EXIT_CODE);
}

TEST_F(IPCTest, 02_child_sends_to_parent) {
  struct mq_attr attr;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 7;

  mqd_t mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  pid_t child_pid = fork();
  if (child_pid == -1) {
    printf("fork error\n");
    assert(0);
  }
  if (child_pid == 0) {
    Mymq_send(mqd, msg3, 3, 3);
    Mymq_send(mqd, msg5, 5, 5);
    Mymq_send(mqd, msg1, 1, 1);
    exit(CUSTOM_SUCCESS_EXIT_CODE);
  }
  sleep(2); /* let child send 3 messages */

  int rc;
  unsigned int prio;
  rc = Mymq_receive(mqd, msg, attr.mq_msgsize, &prio);
  ASSERT_EQ(rc, 5);
  ASSERT_EQ(prio, 5);

  rc = Mymq_receive(mqd, msg, attr.mq_msgsize, &prio);
  ASSERT_EQ(rc, 3);
  ASSERT_EQ(prio, 3);

  rc = Mymq_receive(mqd, msg, attr.mq_msgsize, &prio);
  ASSERT_EQ(rc, 1);
  ASSERT_EQ(prio, 1);

  int exit_status_raw;
  while (waitpid(child_pid, &exit_status_raw, 0) == -1) {
    if (errno == EINTR) {
      printf("EINTR\n");
    }
  }
  ASSERT_EQ(WEXITSTATUS(exit_status_raw), CUSTOM_SUCCESS_EXIT_CODE);
}
