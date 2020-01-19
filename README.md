# POSIX Mac Addons

This is a collection of the POSIX functions which are not available on macOS.

The status of the code: hacky, experimental.

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

**Known limitations:** this implementation uses `sigqueue` for asynchronous
notifications via `mq_notify`. `sigqueue` and the real-time signals
functionality is not available on macOS. This is not a problem if you want to
use `mqueue` with synchronous calls only.

## Founding Principles

- Every port should have at least basic test code coverage but it would be great
  to have tests that ensure the exact behavior of the code on macOS compared to
  the real POSIX equivalents.

- The code is located in the `src` folder with every port in a subfolder. It
  should be possible to use both full `posix-addons` CMake project and each and
  every of the sub-folders standalone.
