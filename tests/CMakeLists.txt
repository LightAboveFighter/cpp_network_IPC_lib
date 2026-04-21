cmake_minimum_required(VERSION 3.10.0)
project(tests VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(tests 
    test_sys_classes.cpp
    ../sys_classes.cpp
)
target_include_directories(tests PRIVATE src)
target_link_libraries(tests PRIVATE gtest_main pthread)

include(CTest)
add_test(NAME sys_classes_tests COMMAND tests)