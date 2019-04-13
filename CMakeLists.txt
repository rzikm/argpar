cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(argpar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# put build executables into build subdirectory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(argpar STATIC ./src/argpar.cpp)
target_include_directories(argpar PUBLIC include)

#Google Test library
include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE) #make it compile on Windows
add_subdirectory(vendor/google/googletest/googletest)
enable_testing()

add_executable(test1 ./test/test1.cpp)
target_link_libraries(test1 gtest gtest_main)
target_link_libraries(test1 argpar)
gtest_discover_tests(test1)

# compile example project
add_executable(example ./example/example.cpp)
target_link_libraries(example argpar)

