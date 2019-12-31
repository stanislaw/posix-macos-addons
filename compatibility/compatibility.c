#include "compatibility.h"

#include <signal.h>

int sigqueue(pid_t pid, int sig, const union sigval value) {
  /// sigqueue does not exit on macOS but it looks like it is enough if we
  /// just send a signal with kill.
  return kill(pid, sig);
}
