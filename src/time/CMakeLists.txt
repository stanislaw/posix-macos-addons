add_library(time-addons posix-mac-time.c)
target_include_directories(time-addons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})