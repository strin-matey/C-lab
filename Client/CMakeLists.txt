# Minimum version for CMake
cmake_minimum_required(VERSION 3.4)

project(Client VERSION 1.0 LANGUAGES C)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Create executable from client.c
add_executable (client src/client.c)
