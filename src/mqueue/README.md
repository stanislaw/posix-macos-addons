# mqueue: POSIX Message Queues implementation based on memory-mapped files

## Changes made to the original code

- The `unpipc.h` file was removed as unnecessary dependency.

- The author's original `err*` helper functions were removed and replaced with
  `printf`.

- The author's original wrapper functions, such as `Mymq_close`, `Mymq_getattr`.
  All calls to these functions were replaced with the normal calls such as
  `mq_close`, `mq_getattr`, etc.

- The queues' memory-mapped files are created in the `tmp` folder. This is to
  allow the queues to be named like: `/queue-1`.
