add_library(rt rt.c)

add_library(posix-mac-stubs INTERFACE)
target_include_directories(posix-mac-stubs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
