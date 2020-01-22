#include "mqueue.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int mq_recv_wait(struct mq_hdr *);

ssize_t mq_receive(mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop) {
  int rc;
  long index;
  int8_t *mptr;
  ssize_t len;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct mymsg_hdr *msghdr;
  struct mq_info *mqinfo;

  mqinfo = (struct mq_info *)mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    printf("mq_receive: magic = %ld\n", mqinfo->mqi_magic);

  mqhdr = mqinfo->mqi_hdr; /* struct pointer */
  mptr = (int8_t *)mqhdr; /* byte pointer */
  attr = &mqhdr->mqh_attr;
  pthread_mutex_lock(&mqhdr->mqh_lock);

  if (maxlen < attr->mq_msgsize) {
    errno = EMSGSIZE;
    goto err;
  }
  if (attr->mq_curmsgs == 0) { /* queue is empty */
    if (mqinfo->mqi_flags & O_NONBLOCK) {
      errno = EAGAIN;
      goto err;
    }
    /* 4wait for a message to be placed onto queue */
    if ((rc = mq_recv_wait(mqhdr)) != 0) {
      errno = rc;
      goto err;
    }
  }

  if ((index = mqhdr->mqh_head) == 0) {
    printf("mq_receive: curmsgs = %ld; head = 0\n", attr->mq_curmsgs);
  }

  msghdr = (struct mymsg_hdr *)&mptr[index];
  mqhdr->mqh_head = msghdr->msg_next; /* new head of list */
  len = msghdr->msg_len;
  memcpy(ptr, msghdr + 1, len); /* copy the message itself */
  if (priop != NULL)
    *priop = msghdr->msg_prio;

  /* 4just read message goes to front of free list */
  msghdr->msg_next = mqhdr->mqh_free;
  mqhdr->mqh_free = index;

  /* 4wake up anyone blocked in mq_send waiting for room */
  if (attr->mq_curmsgs == attr->mq_maxmsg)
    pthread_cond_signal(&mqhdr->mqh_wait);
  attr->mq_curmsgs--;

  pthread_mutex_unlock(&mqhdr->mqh_lock);
  return (len);

err:
  pthread_mutex_unlock(&mqhdr->mqh_lock);
  return (-1);
}
/* end mq_receive */

/* include mq_recv_wait */
static void *mq_wait_thread(void *);
static int pipefd[2];

static int mq_recv_wait(struct mq_hdr *mqhdr) {
  int rc;
  char c;
  pthread_t tid;

  if (pipe(pipefd) == -1)
    return (errno);
  if ((rc = pthread_create(&tid, NULL, mq_wait_thread, mqhdr)) != 0) {
    close(pipefd[0]);
    close(pipefd[1]);
    return (rc);
  }
  /* 4read returns 0 if queue nonempty, else -1 with errno set */
  if ((rc = read(pipefd[0], &c, 1)) != 0)
    rc = errno;
  close(pipefd[0]);
  close(pipefd[1]);
  pthread_cancel(tid); /* must cancel thread if queue still empty */
  return (rc); /* 0 if queue nonempty, else an errno */
}

static void *mq_wait_thread(void *arg) {
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;

  mqhdr = (struct mq_hdr *)arg;
  attr = &mqhdr->mqh_attr;
  while (attr->mq_curmsgs == 0)
    pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);

  close(pipefd[1]); /* queue not empty, close write end of pipe */
  return (NULL);
}
/* end mq_recv_wait */
