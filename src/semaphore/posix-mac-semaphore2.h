#pragma once

#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef dispatch_semaphore_t mac_sem2_t;

int mac_sem2_init(mac_sem2_t *psem, int flags, unsigned count);
int mac_sem2_destroy(mac_sem2_t *psem);
int mac_sem2_post(mac_sem2_t *psem);
int mac_sem2_trywait(mac_sem2_t *psem);
int mac_sem2_wait(mac_sem2_t *psem);
int mac_sem2_timedwait(mac_sem2_t *psem, const struct timespec *abstim);
int mac_sem2_getvalue(mac_sem2_t *sem, int *sval);

#ifdef __cplusplus
}
#endif
