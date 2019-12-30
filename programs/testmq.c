#include <assert.h>
#include "mqueue.h"
#include "unpipc.h"

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

static void sig_alrm(int);
static void sig_usr1(int);
int blocked, sigusr1;

Sigfunc *Signal(int signo, Sigfunc *func) /* for our signal() function */
{
  Sigfunc *sigfunc;

  if ((sigfunc = signal(signo, func)) == SIG_ERR) {
    assert(0 && "Should not reach here");
    err_sys("signal error");
  }

  return (sigfunc);
}

int main(int argc, char **argv) {
  int rc;
  pid_t childpid;
  mqd_t mqd;
  unsigned int prio;
  struct sigevent sigev;
  struct mq_attr attr, info;

  if (argc != 2) {
    err_quit("usage: testmymq <pathname>");
  }

  mq_unlink(argv[1]);
  if ((rc = mq_unlink(argv[1])) == 0 || errno != ENOENT) {
    err_sys("mq_unlink returned %d", rc);
  }

  attr.mq_maxmsg = 4;
  attr.mq_msgsize = 7;
  mqd = Mymq_open(argv[1], O_CREAT | O_EXCL | O_RDWR, FILE_MODE, &attr);

#ifdef notdef
  /* queue is empty; check that mymq_receive is interruptable */
  Signal(SIGALRM, sig_alrm);
  blocked = 1;
  alarm(4);
  if ((rc = mymq_receive(mqd, msg, info.mq_msgsize, &prio)) == 0 ||
      errno != EINTR)
    err_sys("mq_receive returned %d, expected EINTR", rc);
  alarm(0);
  printf("mymq_receive interrupted, as expected\n");

  /* fill queue */
  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg2, 2, 2);
  Mymq_send(mqd, msg3, 3, 3);
  Mymq_send(mqd, msg4, 4, 4);
  Mymq_getattr(mqd, &info);
  if (info.mq_maxmsg != 4 || info.mq_msgsize != 7 || info.mq_flags != 0 ||
      info.mq_curmsgs != 4)
    err_quit("7: maxmsg = %ld, msgsize = %ld, flags = %ld, curmsgs = %ld\n",
             info.mq_maxmsg,
             info.mq_msgsize,
             info.mq_flags,
             info.mq_curmsgs);
  /* queue is full; check that mymq_send is interruptable */
  Signal(SIGALRM, sig_alrm);
  blocked = 1;
  alarm(4);
  printf("about to call mymq_send\n");
  if ((rc = mymq_send(mqd, msg5, 5, 5)) == 0 || errno != EINTR)
    err_sys("mq_send returned %d, expected EINTR", rc);
  alarm(0);
  printf("mymq_send interrupted, as expected\n");
#endif

  /* queue is empty, register for notification */
  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR1;
  Signal(SIGUSR1, sig_usr1);
  Mymq_notify(mqd, &sigev);

  /* verify we cannot register twice */
  if ((rc = mq_notify(mqd, &sigev)) == 0 || errno != EBUSY) {
    err_sys("mq_notify returned %d, expected EBUSY", rc);
  }

  /* now unregister, then reregister */
  Mymq_notify(mqd, NULL);
  Mymq_notify(mqd, &sigev);
  /* verify we cannot register twice */
  if ((rc = mq_notify(mqd, &sigev)) == 0 || errno != EBUSY) {
    err_sys("mq_notify returned %d, expected EBUSY", rc);
  }

  /* make certain child cannot register if we are registered */
  if ((childpid = fork()) == -1) {
    printf("fork error\n");
    assert(0);
  }

  if (childpid == 0) {
    printf("first line of a child\n");
    if ((rc = mq_notify(mqd, &sigev)) == 0 || errno != EBUSY) {
      printf("mq_notify returned %d, expected EBUSY\n", rc);
    }
    printf("last line of a child\n");
    exit(0);
  }

  if (waitpid(childpid, NULL, 0) == -1) {
    perror("perror");
    printf("waitpid error\n");
    assert(0);
  }

  /* send a message and verify SIGUSR1 is delivered */
  sigusr1 = 0;
  Mymq_send(mqd, msg5, 5, 5);
  sleep(1);
  if (sigusr1 != 1)
    err_quit("sigusr1 = %d, expected 1", sigusr1);
  if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 5)
    err_quit("mq_receive returned %d, expected 5", rc);
  if (prio != 5)
    err_quit("mq_receive returned prio %d, expected 5", prio);

  /* send another and make certain another signal is not sent */
  Mymq_send(mqd, msg2, 2, 2);
  sleep(1);
  if (sigusr1 != 1)
    err_quit("sigusr1 = %d, expected 1", sigusr1);
  if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 2)
    err_quit("mq_receive returned %d, expected 2", rc);
  if (prio != 2)
    err_quit("mq_receive returned prio %d, expected 2", prio);

  /* reregister */
  Mymq_notify(mqd, &sigev);

  if ((childpid = fork()) == -1) {
    printf("fork error\n");
    assert(0);
  }
  if (childpid == 0) {
    /* child calls mq_receive, which prevents notification */
    if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 6)
      err_quit("mq_receive returned %d, expected 6", rc);
    if (prio != 6)
      err_quit("mq_receive returned prio %d, expected 6", prio);
    exit(0);
  }
  sleep(2); /* let child block in mq_receive() */
  Mymq_send(mqd, msg6, 6, 6);
  if (waitpid(childpid, NULL, 0) == -1) {
    printf("waitpid error");
    assert(0);
  }
  if (sigusr1 != 1) {
    err_quit("sigusr1 = %d, expected 1", sigusr1);
  }

  /* make certain parent can send messages to child, and vice versa */
  if ((childpid = fork()) == -1) {
    printf("fork error\n");
    assert(0);
  }
  if (childpid == 0) {
    sleep(2); /* let parent send two messages */
    if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 3)
      err_quit("mq_receive returned %d, expected 3", rc);
    if (prio != 3)
      err_quit("mq_receive returned prio %d, expected 3", prio);
    if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 1)
      err_quit("mq_receive returned %d, expected 1", rc);
    if (prio != 1)
      err_quit("mq_receive returned prio %d, expected 1", prio);
    exit(0);
  }
  Mymq_send(mqd, msg1, 1, 1);
  Mymq_send(mqd, msg3, 3, 3);
  if (waitpid(childpid, NULL, 0) == -1) {
    printf("waitpid error");
    assert(0);
  }
  Mymq_getattr(mqd, &info);
  if (info.mq_maxmsg != 4 || info.mq_msgsize != 7 || info.mq_flags != 0 ||
      info.mq_curmsgs != 0)
    err_quit("8: maxmsg = %ld, msgsize = %ld, flags = %ld, curmsgs = %ld\n",
             info.mq_maxmsg,
             info.mq_msgsize,
             info.mq_flags,
             info.mq_curmsgs);

  if ((childpid = fork()) == -1) {
    printf("fork error\n");
    assert(0);
  }
  if (childpid == 0) {
    Mymq_send(mqd, msg3, 3, 3);
    Mymq_send(mqd, msg5, 5, 5);
    Mymq_send(mqd, msg1, 1, 1);
    exit(0);
  }
  sleep(2); /* let child send 3 messages */
  if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 5)
    err_quit("mq_receive returned %d, expected 5", rc);
  if (prio != 5)
    err_quit("mq_receive returned prio %d, expected 5", prio);
  if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 3)
    err_quit("mq_receive returned %d, expected 3", rc);
  if (prio != 3)
    err_quit("mq_receive returned prio %d, expected 3", prio);
  if ((rc = Mymq_receive(mqd, msg, info.mq_msgsize, &prio)) != 1)
    err_quit("mq_receive returned %d, expected 1", rc);
  if (prio != 1)
    err_quit("mq_receive returned prio %d, expected 1", prio);
  if (waitpid(childpid, NULL, 0) == -1) {
    printf("waitpid error");
    assert(0);
  }
  Mymq_getattr(mqd, &info);
  if (info.mq_maxmsg != 4 || info.mq_msgsize != 7 || info.mq_flags != 0 ||
      info.mq_curmsgs != 0)
    err_quit("9: maxmsg = %ld, msgsize = %ld, flags = %ld, curmsgs = %ld\n",
             info.mq_maxmsg,
             info.mq_msgsize,
             info.mq_flags,
             info.mq_curmsgs);

  printf("done\n");
  exit(0);
}

static void sig_usr1(int signo) {
  sigusr1++;
  return;
}

__unused static void sig_alrm(int signo) {
  printf("SIGALRM caught\n");
  return;
}
