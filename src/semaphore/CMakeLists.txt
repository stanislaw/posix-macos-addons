add_library(posix-macos-semaphore
  posix-macos-semaphore.c
  posix-macos-semaphore2.c
)

target_include_directories(posix-macos-semaphore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(posix-macos-semaphore PRIVATE posix-macos-time)

install(TARGETS posix-macos-semaphore)
install(FILES posix-macos-semaphore.h TYPE INCLUDE)
install(FILES posix-macos-semaphore2.h TYPE INCLUDE)
