cmake_minimum_required(VERSION 2.8)

project(AQTools)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

find_package(Boost 1.48.0 COMPONENTS program_options thread filesystem system timer regex REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  $ENV{AQUTIL_SOURCE_DIR}
  $ENV{AQQUERY_RESOLVER_SOURCE_DIR}
  $ENV{AQTESTS_SOURCE_DIR}
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  $ENV{AQUTIL_LIBRARY_DIR}
  $ENV{AQQUERY_RESOLVER_LIBRARY_DIR}
  $ENV{AQTESTS_LIBRARY_DIR}
)

# Mac OS X specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(OperatingSystem "Darwin")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11 -stdlib=libc++ -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++")
    set(CMAKE_SHARED_LINKER_FLAGS "-lc++")
    set(CMAKE_MODULE_LINKER_FLAGS "-lc++")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

file(GLOB_RECURSE source_files aq/*.cpp)
file(GLOB exclude_files aq/aq_tests_batch.cpp)
list(REMOVE_ITEM source_files ${exclude_files})

add_executable(
  aq-tools
  ${source_files}
)

target_link_libraries( 
  aq-tools
  aq-query-resolver
  aq-engine
  aq-util
  ${Boost_LIBRARIES}
)

add_executable(
  aq-tests-batch
  aq/aq_tests_batch.cpp
)

target_link_libraries(
  aq-tests-batch
  aq-tests
  aq-query-resolver
  aq-engine
  aq-util
  ${Boost_LIBRARIES}
  mysqlcppconn-static.a
  /usr/local/mysql/lib/libmysqlclient.a
)
