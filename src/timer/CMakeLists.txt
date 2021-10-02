add_library(posix-macos-timer posix-macos-timer.c)
target_include_directories(posix-macos-timer PUBLIC ${CMAKE_CURRENT_LIST_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../time)

add_dependencies(posix-macos-timer posix-macos-time)

install(TARGETS posix-macos-timer)
install(FILES posix-macos-timer.h TYPE INCLUDE)
