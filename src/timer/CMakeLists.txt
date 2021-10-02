add_library(posix-macos-timer posix-macos-timer.c)
target_include_directories(posix-macos-timer PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(posix-macos-timer PRIVATE posix-macos-time)

install(TARGETS posix-macos-timer)
install(FILES posix-macos-timer.h TYPE INCLUDE)
