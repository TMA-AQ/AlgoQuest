cmake_minimum_required(VERSION 2.8)

project(AQUnitTests)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

find_package(
  Boost 1.50.0 
  COMPONENTS test_exec_monitor unit_test_framework program_options thread filesystem system timer 
  REQUIRED
)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${AQUTIL_SOURCE_DIR}
  ${AQQUERYRESOLVER_SOURCE_DIR}
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  ${AQUTIL_LIBRARY_DIR}
  ${AQQUERYRESOLVER_LIBRARY_DIR}
)

set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11")

file(
  GLOB_RECURSE
  source_files
  src/*.cpp
)

add_executable(
  aq-unitary-tests
  ${source_files}
)

target_link_libraries( 
  aq-unitary-tests
  aq_query_resolver
  aq_util
  ${Boost_LIBRARIES}
)
