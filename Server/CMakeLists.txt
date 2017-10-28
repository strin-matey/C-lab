# Minimum version for CMake
cmake_minimum_required(VERSION 3.0)

project(Server VERSION 1.0 LANGUAGES C)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

add_library(i2c STATIC src/i2c.c)

add_executable(server src/server.c)

target_link_libraries(server LINK_PUBLIC i2c)
