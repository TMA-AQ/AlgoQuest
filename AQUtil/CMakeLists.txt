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

# Linux specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(OperatingSystem "Linux")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -fPIC -std=c++0x -Wall -Werror")
    file(GLOB exclude_files "aq/WIN32FileMapper.*")
    list(REMOVE_ITEM source_files ${exclude_files})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Mac OS X specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(OperatingSystem "Darwin")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11 -stdlib=libc++ -Wall -Werror -DENABLE_AQL_PARSER")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++")
    file(GLOB exclude_files "aq/WIN32FileMapper.*")
    list(REMOVE_ITEM source_files ${exclude_files})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_library(aq-util STATIC ${source_files})

# check-logger
add_executable(check-logger Tests/check_log.cpp)

target_link_libraries(check-logger aq_util ${Boost_LIBRARIES})

# check-file-mapper
add_executable(check-file-mapper Tests/check_file_mapper.cpp)

target_link_libraries(check-file-mapper aq_util ${Boost_LIBRARIES})

# check-parallel-process
add_executable(check-parallel-process Tests/check_parallel_process.cpp)

target_link_libraries(check-parallel-process aq_util ${Boost_LIBRARIES})

# aql-parser
add_executable(aql-parser Tests/aql_parser.cpp)
target_link_libraries(aql-parser aq_util ${Boost_LIBRARIES})
