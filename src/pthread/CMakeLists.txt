add_library(posix-macos-pthread posix-macos-pthread.c)

target_include_directories(posix-macos-pthread PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(posix-macos-pthread PRIVATE posix-macos-time)

install(TARGETS posix-macos-pthread)
install(FILES posix-macos-pthread.h TYPE INCLUDE)
