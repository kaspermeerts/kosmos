CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(kosmos)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
add_definitions(-Wall -Wextra -pedantic -std=c99 -g)
add_definitions(-DROOT_PATH=${CMAKE_SOURCE_DIR})

include_directories(external)
file(GLOB external_sources external/*.c)
add_subdirectory(src)
