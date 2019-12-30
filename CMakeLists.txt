cmake_minimum_required(VERSION 3.13)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wunused -Wpedantic -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address")

add_subdirectory(compatibility)
add_subdirectory(mqueue)
add_subdirectory(posix-message-queue-by-stevens)
add_subdirectory(tests)
