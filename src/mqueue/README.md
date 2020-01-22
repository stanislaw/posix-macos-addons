# mqueue: POSIX Message Queues implementation based on memory-mapped files

## Changes made to the original code

- The `unpipc.h` file was removed as unnecessary dependency.
- The author's original `err*` helper functions were removed and replaced with
`printf`.
 