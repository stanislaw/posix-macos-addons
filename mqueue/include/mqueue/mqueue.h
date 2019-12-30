#pragma once

#include <signal.h>
#include <unistd.h>
#include <pthread/pthread.h>

typedef struct mq_info *mqd_t;

struct mq_attr {
  long mq_flags;   /* message queue flag: 0-NONBLOCK */
  long mq_maxmsg;  /* max number of messages allowed on queue */
  long mq_msgsize; /* max size of a message (in bytes) */
  long mq_curmsgs; /* number of messages currently on queue */
};

/* one mq_hdr{} per queue, at beginning of mapped file */
struct mq_hdr {
  struct mq_attr mqh_attr; /* the queue's attributes */
  long mqh_head;
  long mqh_free;
  long mqh_nwait;
  pid_t mqh_pid;
  struct sigevent mqh_event;
  pthread_mutex_t mqh_lock;
  pthread_cond_t mqh_wait;
};

struct msg_hdr {
  long msg_next;   /* index of next on linked list, must be first member in struct */
  ssize_t msg_len; /* actual length */
  unsigned int msg_prio; /* priority */
};

struct mq_info {
  struct msg_hdr *mqi_hdr;
  long mqi_magic;
  int mqi_flags; /* flags for this process */
};

#define MQI_MAGIC 0x98765432

/* size of message in file is rounded up for alignment */
#define MSGSIZE(i) ((((i) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

mqd_t mq_open(const char *pathname, int oflag, ...);
