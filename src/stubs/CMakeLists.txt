add_library(rt rt.c)

add_library(posix-macos-stubs INTERFACE)
target_include_directories(posix-macos-stubs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
