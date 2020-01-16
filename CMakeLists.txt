cmake_minimum_required(VERSION 3.13)

project(POSIX-Mac-Addons C)

set (CMAKE_CXX_STANDARD 11)

option(NO_FLAGS "Disable usual compilation flags" OFF)
if(NOT DEFINED NO_FLAGS)
  set(CMAKE_C_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
  set(CMAKE_CXX_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address")
endif()

add_subdirectory(src)
add_subdirectory(tests)
