#include "semaphore-mac.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

const int CHILD_STATUS_UNKNOWN = 123;
const int CHILD_STATUS_TERMINATE_FLAG = 124;

typedef struct sem_timedwait_info {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  pthread_t parent;
  struct timespec timeout;
  int wait_status;
  volatile int block_parent_flag;
} sem_timedwait_info;

static void dummy_sighandler(int signo) {
  printf("Dummy signal handler only to allow catching EINTR from sem_wait()\n");
}

void *sem_timedwait_child_thread_entry(void *main_ptr) {
  struct sem_timedwait_info *shared_info = (sem_timedwait_info *)main_ptr;

  // Wait until the timeout or the condition is signaled, whichever comes first.
  int result;
  do {
    pthread_mutex_lock(&shared_info->mutex);
    if (shared_info->wait_status == CHILD_STATUS_TERMINATE_FLAG) {
      goto brk;
    }

    printf("pthread_cond_timedwait start\n");
    result = pthread_cond_timedwait(&shared_info->condition, &shared_info->mutex,
                                    &shared_info->timeout);

    shared_info->wait_status = result;
    pthread_mutex_unlock(&shared_info->mutex);

    printf("pthread_cond_timedwait result: %d\n", result);
    if (result == 0 || result == ETIMEDOUT) {
      goto brk;
    }

    assert(0);

    brk: {
      pthread_mutex_unlock(&shared_info->mutex);
      break;
    }

    assert(0);
  } while (1);

  assert(pthread_kill(shared_info->parent, SIGALRM) == 0);
  printf("signalling parent thread: %s:%d\n", __FILE__, __LINE__);
  shared_info->block_parent_flag = 0;

  return NULL;
}

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
  //  assert(0 && "Should not reach here");
  // Quick test to see if a lock can be immediately obtained.
  int Result;

  do {
    Result = sem_trywait(sem);
    if (Result == 0) {
      return 0;
    }
  } while (Result < 0 && errno == EINTR);

  sem_timedwait_info childThreadInfo;

  pthread_mutex_init(&childThreadInfo.mutex, NULL);
  pthread_cond_init(&childThreadInfo.condition, NULL);
  childThreadInfo.parent = pthread_self();
  childThreadInfo.timeout.tv_sec = abs_timeout->tv_sec;
  childThreadInfo.timeout.tv_nsec = abs_timeout->tv_nsec;

  sig_t OldSigHandler = signal(SIGALRM, SIG_DFL);
  assert(OldSigHandler != SIG_ERR);
  assert(signal(SIGALRM, dummy_sighandler) != SIG_ERR);

  childThreadInfo.wait_status = CHILD_STATUS_UNKNOWN;
  childThreadInfo.block_parent_flag = 1;

  pthread_t ChildThread;
  pthread_create(&ChildThread,
                 NULL,
                 sem_timedwait_child_thread_entry,
                 &childThreadInfo);

  int childStatus = CHILD_STATUS_UNKNOWN;
  // Wait for the semaphore, the timeout to expire, or an unexpected error
  // condition.
  do {
    printf("sem_waiting!.......\n");

    Result = sem_wait(sem);
    printf("Finishing sem_waiting! %d %d\n", Result, errno);

    if (Result < 0) {
      if (errno == EINTR) {
        pthread_mutex_lock(&childThreadInfo.mutex);
        childStatus = childThreadInfo.wait_status;
        childThreadInfo.wait_status = CHILD_STATUS_TERMINATE_FLAG;
        pthread_mutex_unlock(&childThreadInfo.mutex);
        if (childStatus == 0 || childStatus == ETIMEDOUT) {
          break;
        }
      }
      break;
    }
  } while (1);

  int LastError = errno;
  assert(childStatus != CHILD_STATUS_UNKNOWN);
  if (childStatus == 0) {
    Result = 0;
    LastError = 0;
  } else if (childStatus == ETIMEDOUT) {
    Result = -1;
    LastError = ETIMEDOUT;
  } else {
    assert(0);
  }


  pthread_mutex_lock(&childThreadInfo.mutex);
  pthread_cond_signal(&childThreadInfo.condition);
  pthread_mutex_unlock(&childThreadInfo.mutex);
  pthread_join(ChildThread, NULL);
  pthread_cond_destroy(&childThreadInfo.condition);
  pthread_mutex_destroy(&childThreadInfo.mutex);

  printf("Restore previous signal handler.\n");

  // Restore previous signal handler.
  signal(SIGALRM, OldSigHandler);
  while (childThreadInfo.block_parent_flag != 0) {
    printf("waiting : block_parent_flag: %d\n", childThreadInfo.block_parent_flag);
  }

  errno = LastError;

  printf("last error is %d %s\n", errno, strerror(errno));

  return Result;
}
