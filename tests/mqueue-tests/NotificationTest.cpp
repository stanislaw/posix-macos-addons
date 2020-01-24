#include "mqueue.h"

#include <gtest/gtest.h>

#include <iostream>

#define BUFFLEN 100
static char msg[BUFFLEN]; /* receive buffer */

//static char msg0[] = "";
//static char msg1[] = "1";
static char msg2[] = "22";
//static char msg3[] = "333";
//static char msg4[] = "4444";
static char msg5[] = "55555";
static char msg6[] = "666666";
//static char msg7[] = "7777777";
//static char msg8[] = "88888888";

typedef	void Sigfunc(int);
static Sigfunc *Signal(int signo, Sigfunc *func) /* for our signal() function */
{
  Sigfunc *sigfunc;

  if ((sigfunc = signal(signo, func)) == SIG_ERR) {
    assert(0 && "Should not reach here");
    printf("signal error\n");
  }

  return (sigfunc);
}

static const char message_queue_name[] = "/test_mqueue";

static const int CUSTOM_SUCCESS_EXIT_CODE = 47;

class NotificationTest : public ::testing::Test {
protected:
  void SetUp() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }

  void TearDown() override {
    assert(mq_unlink(message_queue_name) == 0 || errno == ENOENT);
  }
};

static int sigusr1 = 0;
static void sig_usr1(int signo) {
  sigusr1++;
  return;
}

TEST_F(NotificationTest, 01_registrationOnlyPossibleOnce) {
  int rc;
  mqd_t mqd;
  struct sigevent sigev;

  mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);
  ASSERT_NE(mqd, (mqd_t)-1);

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR1;
  assert(mq_notify(mqd, &sigev) == 0);

  rc = mq_notify(mqd, &sigev);
  ASSERT_EQ(rc, -1);
  ASSERT_EQ(errno, EBUSY);

  /* now unregister, then reregister */
  assert(mq_notify(mqd, NULL) == 0);
  assert(mq_notify(mqd, &sigev) == 0);

  /* verify we cannot register twice */
  rc = mq_notify(mqd, &sigev);
  ASSERT_EQ(rc, -1);
  ASSERT_EQ(errno, EBUSY);
}

TEST_F(NotificationTest, 02_registrationOnlyPossibleOnce_childCannotRegister) {
  /// Make certain child cannot register if we are registered.
  mqd_t mqd;
  struct sigevent sigev;

  mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, NULL);
  ASSERT_NE(mqd, (mqd_t)-1);

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR1;
  assert(mq_notify(mqd, &sigev) == 0);

  pid_t childpid = fork();
  if ((childpid) == -1) {
    printf("fork error\n");
    assert(0);
  }

  if (childpid == 0) {
    if ((mq_notify(mqd, &sigev)) == 0 || errno != EBUSY) {
      exit(1);
    }
    exit(0);
  }

  int exit_status_raw;
  while (waitpid(childpid, &exit_status_raw, 0) == -1) {
    if (errno == EINTR) {
      printf("EINTR\n");
    }
  }

  ASSERT_EQ(WEXITSTATUS(exit_status_raw), 0);
}

TEST_F(NotificationTest, 03_sendAMessageResultsInSIGUSR1_butOnlyOnce) {
  /// send a message and verify SIGUSR1 is delivered
  int rc;
  mqd_t mqd;
  unsigned int prio;
  struct sigevent sigev;

  struct mq_attr attr;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 7;

  mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR1;
  Signal(SIGUSR1, sig_usr1);
  assert(mq_notify(mqd, &sigev) == 0);

  sigusr1 = 0;

  /// Send a message and verify SIGUSR1 is delivered.
  mq_send(mqd, msg5, 5, 5);
  sleep(1);

  ASSERT_EQ(sigusr1, 1);

  rc = mq_receive(mqd, msg, attr.mq_msgsize, &prio);
  ASSERT_EQ(rc, 5);
  ASSERT_EQ(prio, 5);

  /// Send another and make certain another signal is not sent.
  mq_send(mqd, msg2, 2, 2);
  sleep(1);
  ASSERT_EQ(sigusr1, 1);
  rc = mq_receive(mqd, msg, attr.mq_msgsize, &prio);
  ASSERT_EQ(rc, 2);
  ASSERT_EQ(prio, 2);

  Signal(SIGUSR1, NULL);
}

TEST_F(NotificationTest, 04_USR1_is_not_delivered_if_blocked_by_receive) {
  /// send a message and verify SIGUSR1 is not delivered if a child process
  /// is blocking by mq_receive()
  mqd_t mqd;
  unsigned int prio;
  struct sigevent sigev;

  struct mq_attr attr;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 7;

  mqd =
    mq_open(message_queue_name, O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);
  ASSERT_NE(mqd, (mqd_t)-1);

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR1;
  Signal(SIGUSR1, sig_usr1);
  mq_notify(mqd, &sigev);

  sigusr1 = 0;

  pid_t childpid = fork();
  if (childpid == -1) {
    printf("fork error\n");
    assert(0);
  }
  if (childpid == 0) {
    int rc;
    /// Child calls mq_receive, which prevents notification.
    if ((rc = mq_receive(mqd, msg, attr.mq_msgsize, &prio)) != 6) {
      printf("mq_receive returned %d, expected 6\n", rc);
    }
    if (prio != 6) {
      printf("mq_receive returned prio %d, expected 6\n", prio);
    }
    exit(CUSTOM_SUCCESS_EXIT_CODE);
  }

  sleep(2); /* let child block in mq_receive() */
  mq_send(mqd, msg6, 6, 6);

  int exit_status_raw;
  while (waitpid(childpid, &exit_status_raw, 0) == -1) {
    if (errno == EINTR) {
      printf("EINTR\n");
    }
  }
  ASSERT_EQ(WEXITSTATUS(exit_status_raw), CUSTOM_SUCCESS_EXIT_CODE);

  ASSERT_EQ(sigusr1, 0);

  Signal(SIGUSR1, NULL);
}
