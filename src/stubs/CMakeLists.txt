add_library(rt rt.c)

add_library(posix-mac-stubs posix-mac-stubs.c)
target_include_directories(posix-mac-stubs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
