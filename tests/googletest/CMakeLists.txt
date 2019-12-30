message(STATUS "googletest> Creating google-test target...")

set(google_test_include_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")


file(GLOB_RECURSE google_test_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(google_test_sources src/gtest-all.cc src/gtest_main.cc)

add_library(google-test STATIC ${google_test_sources} ${google_test_headers})
target_include_directories(google-test PUBLIC ${google_test_include_dirs})
