add_subdirectory(mqueue)
add_subdirectory(pthread)
add_subdirectory(semaphore)
add_subdirectory(time)
add_subdirectory(timer)
add_subdirectory(stubs)

add_library(posix-mac-addons INTERFACE)

target_link_libraries(posix-mac-addons
  INTERFACE
  mqueue
  posix-mac-pthread
  posix-mac-semaphore
  posix-mac-stubs
  posix-mac-time
  posix-mac-timer
  rt
)
