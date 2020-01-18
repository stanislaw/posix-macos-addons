cmake_minimum_required(VERSION 3.13)

project(POSIX-Mac-Addons C)

set (CMAKE_CXX_STANDARD 11)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(NO_FLAGS "Disable usual compilation flags" OFF)
  if(NOT NO_FLAGS)
    set(CMAKE_C_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
    set(CMAKE_CXX_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address")
  else()
    message("NO_FLAGS is provided: skipping strict compilation flags.")
  endif()
endif()

add_subdirectory(src)
add_subdirectory(tests)
