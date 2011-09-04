CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(kosmos)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src)
add_definitions(-Wall -Wextra -pedantic -std=c99 -g)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
