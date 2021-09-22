add_library(posix-macos-time posix-macos-time.c)
target_include_directories(posix-macos-time PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS posix-macos-time)
install(FILES posix-macos-time.h TYPE INCLUDE)
