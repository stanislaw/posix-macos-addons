add_library(posix-mac-semaphore posix-mac-semaphore.c posix-mac-semaphore2.c)
target_include_directories(posix-mac-semaphore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
