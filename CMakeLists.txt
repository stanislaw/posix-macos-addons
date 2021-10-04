cmake_minimum_required(VERSION 3.13)

project(POSIX-Mac-Addons C)

set (CMAKE_CXX_STANDARD 11)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(NO_FLAGS "Disable usual compilation flags" OFF)
  option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
  if(NOT NO_FLAGS)
    set(CMAKE_C_FLAGS "-Werror -Wall -Wunused -Wpedantic")
    set(CMAKE_CXX_FLAGS "-Werror -Wall -Wunused -Wpedantic")

    execute_process(
      COMMAND xcrun -show-sdk-path
      OUTPUT_VARIABLE SDKROOT
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_OSX_SYSROOT ${SDKROOT})
    # set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX10.9.sdk)

    if(ENABLE_ASAN)
      set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address")
      set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address")
    endif()
  else()
    message("NO_FLAGS is provided: skipping strict compilation flags.")
  endif()
endif()

add_subdirectory(src)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_subdirectory(tests)
endif()