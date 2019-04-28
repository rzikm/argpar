cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(argpar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# put build executables into build subdirectory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(argpar INTERFACE)
target_include_directories(argpar INTERFACE include)
#target_sources(argpar INTERFACE ./include/argpar/argpar.h)

# Google Test library
include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE) #make it compile on Windows
add_subdirectory(vendor/google/googletest/googletest)
enable_testing()

# Add the tests to combined executable
add_executable(tests ./test/test1.cpp ./test/test2.cpp ./test/test3.cpp)
target_link_libraries(tests gtest gtest_main)
target_link_libraries(tests argpar)
gtest_discover_tests(tests)

# compile example project
add_executable(example ./example/example.cpp)
target_link_libraries(example argpar)

