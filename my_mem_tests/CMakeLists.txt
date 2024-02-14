cmake_minimum_required(VERSION 3.13...3.13)
project(my_mem_test_perf VERSION 0.1.0 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

add_executable(my_mem_test_perf my_mem_utils_perf.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
