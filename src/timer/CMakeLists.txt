add_library(posix-macos-timer posix-macos-timer.c)
target_include_directories(posix-macos-timer STATIC PUBLIC ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS posix-macos-timer)
install(FILES posix-macos-timer.h TYPE INCLUDE)
