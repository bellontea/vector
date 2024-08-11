cmake_minimum_required(VERSION 3.28)
project(containers)

set(CMAKE_CXX_STANDARD 20)

add_executable(containers main.cpp
        vector.h)

enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(test_vector test.cpp vector.h)
target_link_libraries(test_vector GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_vector)
