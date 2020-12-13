#ifndef _POSIX_MACOS_MQUEUE_H_
#define _POSIX_MACOS_MQUEUE_H_

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/// TODO: macOS should have this defined
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef struct mq_info *mqd_t; /* opaque datatype */

struct mq_attr {
  long mq_flags; /* message queue flag: O_NONBLOCK */
  long mq_maxmsg; /* max number of messages allowed on queue */
  long mq_msgsize; /* max size of a message (in bytes) */
  long mq_curmsgs; /* number of messages currently on queue */
};

/* one mq_hdr{} per queue, at beginning of mapped file */
struct mq_hdr {
  struct mq_attr mqh_attr; /* the queue's attributes */
  long mqh_head; /* index of first message */
  long mqh_free; /* index of first free message */
  long mqh_nwait; /* #threads blocked in mq_receive() */
  pid_t mqh_pid; /* nonzero PID if mqh_event set */
  struct sigevent mqh_event; /* for mq_notify() */
  pthread_mutex_t mqh_lock; /* mutex lock */
  pthread_cond_t mqh_wait; /* and condition variable */
};

/* one mymsg_hdr{} at the front of each message in the mapped file */
struct mymsg_hdr {
  long msg_next; /* index of next on linked list */
  /* msg_next must be first member in struct */
  ssize_t msg_len; /* actual length */
  unsigned int msg_prio; /* priority */
};

/* one mq_info{} malloc'ed per process per mq_open() */
struct mq_info {
  struct mq_hdr *mqi_hdr; /* start of mmap'ed region */
  long mqi_magic; /* magic number if open */
  int mqi_flags; /* flags for this process */
};
#define MQI_MAGIC 0x98765432

/* size of message in file is rounded up for alignment */
#define MSGSIZE(i) ((((i) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

int mq_close(mqd_t);
int mq_getattr(mqd_t, struct mq_attr *);
int mq_notify(mqd_t, const struct sigevent *);
mqd_t mq_open(const char *, int, ...);
ssize_t mq_receive(mqd_t, char *, size_t, unsigned int *);
int mq_send(mqd_t, const char *, size_t, unsigned int);
int mq_setattr(mqd_t, const struct mq_attr *, struct mq_attr *);
int mq_unlink(const char *name);
int mq_timedsend(mqd_t mqdes,
                 const char *msg_ptr,
                 size_t msg_len,
                 unsigned msg_prio,
                 const struct timespec *abs_timeout);
ssize_t mq_timedreceive(mqd_t mqdes,
                        char *msg_ptr,
                        size_t msg_len,
                        unsigned *msg_prio,
                        const struct timespec *abs_timeout);

#ifdef __cplusplus
}
#endif

#endif /* _POSIX_MACOS_MQUEUE_H_ */
