cmake_minimum_required(VERSION 3.8)

project(Nimata)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast -g3 -Wall -Wextra -Wconversion -Wpedantic -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused-parameter -Wdeprecated")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(NIMATA_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/)

add_executable(Nimata
    ${NIMATA_SOURCES_DIR}/main.cpp 
    # ${NIMATA_SOURCES_DIR}/odr.cpp
)