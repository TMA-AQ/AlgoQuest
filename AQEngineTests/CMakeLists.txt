cmake_minimum_required(VERSION 2.8)

project(AQEngineTests)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

find_package(Boost 1.50.0 COMPONENTS program_options thread filesystem system timer python REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  $ENV{AQUTIL_SOURCE_DIR}
  $ENV{AQQUERY_RESOLVER_SOURCE_DIR}
  /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  $ENV{AQUTIL_LIBRARY_DIR}
  $ENV{AQQUERY_RESOLVER_LIBRARY_DIR}
  /System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7
)

set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11")

file(
  GLOB_RECURSE
  source_files
  aq/*.cpp
)

add_executable(
  aq-tests
  ${source_files}
)

target_link_libraries( 
  aq-tests
  aq_query_resolver
  aq_util
  ${Boost_LIBRARIES}
)

file(GLOB_RECURSE py_bind_src python_binding/*.cpp)
file(GLOB exclude_files "python_binding/dllmain.cpp")
list(REMOVE_ITEM py_bind_src ${exclude_files})
add_library(AlgoQuestDB SHARED ${source_files} ${py_bind_src})
target_link_libraries(AlgoQuestDB aq_query_resolver aq_util ${Boost_LIBRARIES} python)
set_target_properties(AlgoQuestDB PROPERTIES PREFIX '')
set_target_properties(AlgoQuestDB PROPERTIES SUFFIX .so)
