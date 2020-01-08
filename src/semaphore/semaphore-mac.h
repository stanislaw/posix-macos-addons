#pragma once

#include <pthread/pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * The implementation is selected from a number implementations available
 * on the "POSIX Semaphores on Mac OS X: sem_timedwait alternative" thread:
 * https://stackoverflow.com/a/37324520/598057
 */
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

#ifdef __cplusplus
}
#endif
