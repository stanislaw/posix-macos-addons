set(SOURCES
  src/mq_close.c
  src/mq_getattr.c
  src/mq_notify.c
  src/mq_open.c
  src/mq_receive.c
  src/mq_send.c
  src/mq_setattr.c
  src/mq_timedreceive.c
  src/mq_timedsend.c
  src/mq_unlink.c
)

add_library(mqueue ${SOURCES})
target_include_directories(mqueue PUBLIC include include/mqueue)
target_link_libraries(mqueue rt)
