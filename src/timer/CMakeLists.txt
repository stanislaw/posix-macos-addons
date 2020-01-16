
add_library(mac-timer posix-mac-timer.c)
target_include_directories(mac-timer PUBLIC ${CMAKE_CURRENT_LIST_DIR})