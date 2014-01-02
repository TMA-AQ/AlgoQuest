cmake_minimum_required(VERSION 2.8)

project(AQUtil)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

# set(Boost_DEBUG 1)
find_package(Boost 1.48.0 COMPONENTS program_options thread filesystem system timer REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
)

file(
  GLOB
  source_files
  aq/*.cpp
  aq/*.h
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # Linux specific code
    SET(OperatingSystem "Linux")

    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++0x -Wall")

    file(GLOB exclude_files "aq/WIN32FileMapper.*")
    list(REMOVE_ITEM source_files ${exclude_files})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    # Linux specific code
    SET(OperatingSystem "Darwin")

    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11")

    file(GLOB exclude_files "aq/WIN32FileMapper.*")
    list(REMOVE_ITEM source_files ${exclude_files})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    # Linux specific code
    SET(OperatingSystem "FreeBSD")

    set(CMAKE_CXX_COMPILER "g++46")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++0x")

    file(GLOB exclude_files "aq/WIN32FileMapper.*")
    list(REMOVE_ITEM source_files ${exclude_files})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

add_library(
  aq_util
  STATIC
  ${source_files}
)

# check-logger
add_executable(
  check-logger
  Tests/check_log.cpp
)

target_link_libraries( 
  check-logger
  aq_util
  ${Boost_LIBRARIES}
)

# check-file-mapper
add_executable(
  check-file-mapper
  Tests/check_file_mapper.cpp
)

target_link_libraries( 
  check-file-mapper
  aq_util
  ${Boost_LIBRARIES}
)

# check-parallel-process
add_executable(
  check-parallel-process
  Tests/check_parallel_process.cpp
)

target_link_libraries( 
  check-parallel-process
  aq_util
  ${Boost_LIBRARIES}
)
