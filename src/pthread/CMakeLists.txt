add_library(posix-macos-pthread posix-macos-pthread.c)
target_include_directories(posix-macos-pthread PUBLIC ${CMAKE_CURRENT_LIST_DIR})
