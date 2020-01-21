project(POSIX-Mac-Addons-Tests C CXX)

add_subdirectory(googletest)

set(tests-unit-sources
  mqueue-tests/BasicTest.cpp
  mqueue-tests/NotificationTest.cpp
  mqueue-tests/IPCTest.cpp
  mqueue-tests/TimedSendReceiveTest.cpp
  pthread-tests/Pthread_SetSchedPrio_Test.cpp
  semaphore-tests/Semaphore_Test.cpp
  semaphore-tests/Semaphore2_Test.cpp
  semaphore-tests/Semaphore2_Debug_Test.cpp
  time-tests/NanoSleepTest.cpp
  time-tests/NanoSleep_TimeDiff_Test.cpp
  timer-tests/TimerTest.cpp
)

add_executable(tests-unit ${tests-unit-sources})

target_include_directories(tests-unit PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/googletest/include
)

target_link_libraries(tests-unit
  google-test
  pthread

  posix-mac-addons

  ${CMAKE_THREAD_LIBS_INIT}
)
