add_subdirectory(googletest)

set(tests-unit-sources
  BasicTest.cpp
  SIGUSR1_Test.cpp
  IPC_Test.cpp
)

add_executable(tests-unit ${tests-unit-sources})

target_include_directories(tests-unit PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/googletest/include
)

target_link_libraries(tests-unit
  google-test
  pthread

  mqueue
  compatibility

  ${CMAKE_THREAD_LIBS_INIT}
)
