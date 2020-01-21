#pragma once

#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef dispatch_semaphore_t mac_sem2_t;

int mac_sem2_debug_init(mac_sem2_t *psem, int flags, unsigned count);
int mac_sem2_debug_destroy(mac_sem2_t *psem);
int mac_sem2_debug_post(mac_sem2_t *psem);
int mac_sem2_debug_trywait(mac_sem2_t *psem);
int mac_sem2_debug_wait(mac_sem2_t *psem);
int mac_sem2_debug_timedwait(mac_sem2_t *psem, const struct timespec *abstim);
int mac_sem2_debug_getvalue(mac_sem2_t *sem, int *sval);

#ifdef __cplusplus
}
#endif
