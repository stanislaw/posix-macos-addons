#include "semaphore-mac.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

int mac_sem_init(mac_sem_t *psem, int flags, unsigned count) {
  int result;

  if (psem == NULL) {
    return -1;
  }
  result = pthread_mutex_init(&psem->count_lock, NULL);
  if (result != 0) {
    return result;
  }
  result = pthread_cond_init(&psem->count_bump, NULL);
  if (result != 0) {
    pthread_mutex_destroy(&psem->count_lock);
    return result;
  }
  psem->count = count;
  return 0;
}

int mac_sem_destroy(mac_sem_t *psem) {
  mac_sem_t *poldsem;

  if (!psem) {
    return EINVAL;
  }
  poldsem = (mac_sem_t *)psem;

  pthread_mutex_destroy(&poldsem->count_lock);
  pthread_cond_destroy(&poldsem->count_bump);
  return 0;
}

int mac_sem_post(mac_sem_t *psem) {
  mac_sem_t *pxsem;
  int result, xresult;

  if (!psem) {
    return EINVAL;
  }
  pxsem = (mac_sem_t *)psem;

  result = pthread_mutex_lock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  pxsem->count = pxsem->count + 1;

  xresult = pthread_cond_signal(&pxsem->count_bump);

  result = pthread_mutex_unlock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  if (xresult) {
    errno = xresult;
    return -1;
  }
  return 0;
}

int mac_sem_trywait(mac_sem_t *psem) {
  mac_sem_t *pxsem;
  int result, xresult;

  if (!psem) {
    return EINVAL;
  }
  pxsem = (mac_sem_t *)psem;

  result = pthread_mutex_lock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  xresult = 0;

  if (pxsem->count > 0) {
    pxsem->count--;
  } else {
    xresult = EAGAIN;
  }
  result = pthread_mutex_unlock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  if (xresult) {
    errno = xresult;
    return -1;
  }
  return 0;
}

int mac_sem_wait(mac_sem_t *psem) {
  mac_sem_t *pxsem;
  int result, xresult;

  if (!psem) {
    return EINVAL;
  }
  pxsem = (mac_sem_t *)psem;

  result = pthread_mutex_lock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  xresult = 0;

  if (pxsem->count == 0) {
    xresult = pthread_cond_wait(&pxsem->count_bump, &pxsem->count_lock);
  }
  if (!xresult) {
    if (pxsem->count > 0) {
      pxsem->count--;
    }
  }
  result = pthread_mutex_unlock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  if (xresult) {
    errno = xresult;
    return -1;
  }
  return 0;
}

int mac_sem_timedwait(mac_sem_t *psem, const struct timespec *abstim) {
  mac_sem_t *pxsem;
  int result, xresult;

  if (psem == NULL) {
    return EINVAL;
  }
  pxsem = (mac_sem_t *)psem;

  result = pthread_mutex_lock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  xresult = 0;

  if (pxsem->count == 0) {
    xresult =
      pthread_cond_timedwait(&pxsem->count_bump, &pxsem->count_lock, abstim);
  }
  if (xresult == 0) {
    if (pxsem->count > 0) {
      pxsem->count--;
    }
  }
  result = pthread_mutex_unlock(&pxsem->count_lock);
  if (result) {
    return result;
  }
  if (xresult) {
    errno = xresult;
    return -1;
  }
  return 0;
}
