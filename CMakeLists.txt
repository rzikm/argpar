cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(argpar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(argpar STATIC ./src/argpar.cpp)

# compile example project
add_executable(example ./example/example.cpp)
target_link_libraries(example argpar)
