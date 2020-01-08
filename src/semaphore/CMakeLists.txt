add_library(posix-mac-semaphore semaphore-mac.c)
target_include_directories(posix-mac-semaphore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
