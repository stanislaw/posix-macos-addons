add_library(posix-macos-pthread STATIC posix-macos-pthread.c)
target_include_directories(posix-macos-pthread PUBLIC ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS posix-macos-pthread)
install(FILES posix-macos-pthread.h TYPE INCLUDE)
