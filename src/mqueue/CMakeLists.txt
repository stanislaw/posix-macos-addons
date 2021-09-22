set(SOURCES
  src/mq_close.c
  src/mq_getattr.c
  src/mq_internal_fs.c
  src/mq_notify.c
  src/mq_open.c
  src/mq_receive.c
  src/mq_send.c
  src/mq_setattr.c
  src/mq_timedreceive.c
  src/mq_timedsend.c
  src/mq_unlink.c
)

add_library(mqueue STATIC ${SOURCES})
target_include_directories(mqueue PUBLIC include/mqueue)
target_link_libraries(mqueue rt)

install(TARGETS mqueue)
install(FILES include/mqueue/mqueue.h TYPE INCLUDE)
