add_library(posix-mac-pthread pthread-mac.c)
target_include_directories(posix-mac-pthread PUBLIC ${CMAKE_CURRENT_LIST_DIR})
