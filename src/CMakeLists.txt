cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(libgpif)

# Build Release by default
IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# Set CFLAGS
SET(CMAKE_C_FLAGS "-O2 -pedantic -Wall")
SET(CMAKE_C_FLAGS_DEBUG "-g")
SET(CMAKE_C_FLAGS_RELEASE "")

add_subdirectory(lib)

# Build the test suites only in debug mode
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_SUBDIRECTORY(test)
ENDIF()

