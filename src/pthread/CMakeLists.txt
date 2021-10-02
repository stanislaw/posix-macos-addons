add_library(posix-macos-pthread posix-macos-pthread.c)
target_include_directories(posix-macos-pthread PUBLIC ${CMAKE_CURRENT_LIST_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../time)

add_dependencies(posix-macos-pthread posix-macos-time)

install(TARGETS posix-macos-pthread)
install(FILES posix-macos-pthread.h TYPE INCLUDE)
