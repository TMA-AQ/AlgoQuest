cmake_minimum_required(VERSION 2.8)

project(AQQueryResolver)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

find_package(Boost 1.50.0 COMPONENTS program_options thread filesystem system timer REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${AQUTIL_SOURCE_DIR}
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  ${AQUTIL_LIBRARY_DIR}
)

set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11")

file(
  GLOB_RECURSE
  source_files
  aq/*.cpp
)

file(
  GLOB_RECURSE
  to_remove
  aq/parser/lex.yy.cpp
)

list(REMOVE_ITEM source_files ${to_remove})

add_library(
  aq_query_resolver
  STATIC
  ${source_files}
)

add_executable(
  aq-tools
  Tools/aq_tools.cpp
)

target_link_libraries( 
  aq-tools
  aq_query_resolver
  aq_util
  ${Boost_LIBRARIES}
)
