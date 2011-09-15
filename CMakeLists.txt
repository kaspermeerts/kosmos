CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(kosmos)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
add_definitions(-Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes
-Wshadow -pedantic -std=c99 -g -O1 -fstrict-aliasing -Wstrict-aliasing)
add_definitions(-DROOT_PATH=${CMAKE_SOURCE_DIR})

include_directories(external/ralloc)
include_directories(external/rply)

add_subdirectory(external)
add_subdirectory(src)
