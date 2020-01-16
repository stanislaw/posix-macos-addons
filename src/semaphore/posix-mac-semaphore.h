#pragma once

#include <pthread/pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  pthread_mutex_t count_lock;
  pthread_cond_t count_bump;
  unsigned count;
} mac_sem_t;

int mac_sem_init(mac_sem_t *psem, int flags, unsigned count);
int mac_sem_destroy(mac_sem_t *psem);
int mac_sem_post(mac_sem_t *psem);
int mac_sem_trywait(mac_sem_t *psem);
int mac_sem_wait(mac_sem_t *psem);
int mac_sem_timedwait(mac_sem_t *psem, const struct timespec *abstim);

#ifdef __cplusplus
}
#endif
