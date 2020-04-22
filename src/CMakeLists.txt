add_subdirectory(mqueue)
add_subdirectory(pthread)
add_subdirectory(semaphore)
add_subdirectory(time)
add_subdirectory(timer)
add_subdirectory(stubs)

add_library(posix-macos-addons INTERFACE)

target_link_libraries(posix-macos-addons
  INTERFACE
  mqueue
  posix-macos-pthread
  posix-macos-semaphore
  posix-macos-stubs
  posix-macos-time
  posix-macos-timer
  rt
)
