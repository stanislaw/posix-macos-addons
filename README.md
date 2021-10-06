# POSIX addons for macOS

This is a collection of the POSIX functions which are not available on macOS.

The status of the code: hacky, experimental.

![](https://github.com/stanislaw/posix-macos-addons/workflows/Tests%20on%20macOS/badge.svg?branch=main)

## Table of Contents

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Disclaimer](#disclaimer)
- [Background](#background)
- [Founding Principles](#founding-principles)
- [Contents](#contents)
  - [mqueue (POSIX message queues)](#mqueue-posix-message-queues)
  - [pthread](#pthread)
  - [semaphore](#semaphore)
  - [time](#time)
  - [timer](#timer)
- [Known experimental uses](#known-experimental-uses)
- [Not implemented yet](#not-implemented-yet)
- [Similar repositories](#similar-repositories)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Disclaimer

Please note that the exact implementation of the same functionality for every
POSIX function is not always possible because certain bigger parts simply do not
exist on macOS and implementing them would be a much serious effort. The outcome
of this fact is that most of the functions in this repository are
**emulations**: the code follows all or the most of the original API and tries
to do the job like specified on the Man pages but there is no guarantee that you
get a 100% Linux behavior. Please check the Contents for the specific details
below.

## Background

There are two opinions on the internet:

1. macOS should match Linux 100% so if something POSIX exists on Linux it should
   be available on macOS.

2. macOS has enough of POSIX. The uninimplemented parts are actually POSIX
   extensions so it is ok to not have them.

We can agree or disagree with either or both of these points of view but
meanwhile we also need to run some Linux software on macOS and sometimes at
work.

The idea behind this repository is to finally have a single place for these
marginal POSIX functions and the hope is that they should be a good-enough
replacement for their real POSIX equivalents.

## Founding Principles

- This repo is based on the
  [Cunningham's Law](https://en.wikipedia.org/wiki/Ward_Cunningham#Cunningham%27s_Law):
  the code in this repository can be wrong but at least it is there which makes
  it available for use and further improvement.

- Every port should have at least basic test code coverage but it would be great
  to have tests that ensure the exact behavior of the code on macOS compared to
  the real POSIX equivalents.

- The code is located in the `src` folder with every port in a subfolder. It
  should be possible to use both full `posix-macos-addons` CMake project and each
  and every of the sub-folders standalone.

## Contents

### mqueue (POSIX message queues)

The mqueue does not exist on macOS.

The implementation is adapted from the book
["UNIX Network Programming, Volume 2"](http://www.kohala.com/start/unpv22e/unpv22e.html)
by W. Richard Stevens. The book provides source code where there is an
implementation based on the memory-mapped files written by the author. The code
was adapted to macOS and all of the original author's test programs were
converted to a Google Test suite.

**Status:** the code seems to work, but needs some further cleanups.

#### Known limitations

1. this implementation uses `sigqueue` for asynchronous notifications via
   `mq_notify`. `sigqueue` and the real-time signals functionality is not
   available on macOS. This is not a problem if you want to use `mqueue` with
   synchronous calls only.

2. The Linux implementation uses a virtual file system while memory-mapped files
   on macOS are created in a user's file system. This means that the naming
   conventions are different: for example you cannot create mqueue in your root
   directory like `/queue-1`.

### pthread

`pthread_setschedprio` function is implemented as a call to the
`pthread_setschedparam` function which is available on macOS.

### semaphore

Unnamed semaphores: `sem_init`, `sem_wait`, `sem_post`, `sem_destroy` are
deprecated on macOS. Additionally, the `sem_timedwait` does not exist.

There are two implementations: `mac_sem_*` and `mac_sem2_*`.

The `mac_sem_*` implementation is based on `pthread_mutex` and `pthread_cond`
and is taken from this StackOverflow thread with minor adaptations:
[POSIX Semaphores on Mac OS X: sem_timedwait alternative](https://stackoverflow.com/a/48778462/598057).

The `mac_sem2_*` implementation is based on GCD semaphores.

#### Known issue

GCD semaphores crash with `BUG IN CLIENT OF LIBDISPATCH` when you destroy a
semaphore which is being waited on this is why there is a debug version of the
second implementation: `mac_sem2_debug_*` where there is a layer that tracks how
many times a certain semaphore was waited/posted to release the semaphore
manually when it gets destroyed. This behavior was needed to make one important
project's tests to pass without crashing.

### time

The `clock_nanosleep` is implemented as a call to `nanosleep`. The code is
inspired by this implementation:
[btest-opensource/timing_mach.h](https://github.com/samm-git/btest-opensource/blob/236d9a79b03c5c696832a1f5134c792a7ecb3ef1/timing_mach.h).

Possible issue:
[nasa/osal#337](https://github.com/nasa/osal/issues/337#issuecomment-570807421).

### timer

A rather limited implementation based on the GCD timers.

The implementation has an additional method `timer_poll` to simulate polling for
the timer's ticks to substitute waiting on `sigwait` to receive a signal from a
timer when configured with `SIGEV_SIGNAL`.

## Known experimental uses

This code is being tested as part of the experimental macOS ports of the
following projects:

- [nasa/osal](https://github.com/nasa/osal)

- [DLR-RY/OUTPOST](https://github.com/DLR-RY/outpost-core)

## Similar repositories

- [libUnixToOSX](https://github.com/cooljeanius/libUnixToOSX)
- https://github.com/ChisholmKyle/PosixMachTiming
- https://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
- https://speakerdeck.com/jj1bdx/how-i-discover-a-working-implementation-of-clock-nanosleep-for-macos-in-cpan-time-hires?slide=8
- [macports-legacy-support](https://github.com/macports/macports-legacy-support)

These are similar repositories. We haven't checked these implementations yet,
there might be better implementations hiding.
