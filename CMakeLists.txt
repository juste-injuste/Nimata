cmake_minimum_required(VERSION 3.8)

project(Nimata)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall -Wextra -pedantic -Wpedantic -O3 -g3")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(EXOARM_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/)

add_executable(Nimata ${EXOARM_SOURCES_DIR}/main.cpp)