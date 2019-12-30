#include "compatibility.h"

#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

int sigqueue(pid_t pid, int sig, const union sigval value) {
  printf("pid: %d, sig: %d, sigval:%d\n", pid, sig, value.sival_int);
  return kill(pid, sig);

//  siginfo_t *si;
//  const size_t sz = sizeof(*si);
//  si = calloc(1, sz);
//  si->si_signo = SIGWINCH;
//  si->si_code = SI_QUEUE;
//  si->si_pid = getpid();
//  si->si_uid = getuid();
//  syscall(178, getpid(), SIGWINCH, si);
//  free(si);
//  fprintf(stderr, "Done.\n");
//  return 0;
}
