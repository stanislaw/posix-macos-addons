add_executable(mqueue-example main.c)
target_link_libraries(mqueue-example mqueue-mac)

set(CMAKE_C_FLAGS "-Werror -Wall -Wunused -Wpedantic")
