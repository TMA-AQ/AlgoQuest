cmake_minimum_required(VERSION 2.8)

project(AQEngineTests)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.48.0 COMPONENTS program_options thread filesystem system timer python regex REQUIRED)

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

# Mac OS X specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(OperatingSystem "Darwin")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11 -stdlib=libc++ -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++")
    set(CMAKE_SHARED_LINKER_FLAGS "-lc++")
    set(CMAKE_MODULE_LINKER_FLAGS "-lc++")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

file(GLOB_RECURSE source_files aq/*.cpp)
file(GLOB_RECURSE exclude_files aq/Where*.cpp aq/AQEngineTests.cpp)
list(REMOVE_ITEM source_files ${exclude_files})

# aq-tests static library
add_library(aq-tests STATIC ${source_files})

# aq-tests shared library
add_library(aq-tests-shared SHARED ${source_files})
target_link_libraries(aq-tests-shared aq-query-resolver aq-engine aq-util ${Boost_LIBRARIES})
set_target_properties(aq-tests-shared PROPERTIES OUTPUT_NAME "aq-tests")

# file(GLOB_RECURSE py_bind_src python_binding/*.cpp)
# file(GLOB exclude_files "python_binding/dllmain.cpp")
# list(REMOVE_ITEM py_bind_src ${exclude_files})
# add_library(AlgoQuestDB SHARED ${source_files} ${py_bind_src})
# target_link_libraries(AlgoQuestDB aq_query_resolver aq_util ${Boost_LIBRARIES} python)
# set_target_properties(AlgoQuestDB PROPERTIES PREFIX '')
# set_target_properties(AlgoQuestDB PROPERTIES SUFFIX .so)
