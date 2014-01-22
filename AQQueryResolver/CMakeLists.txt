cmake_minimum_required(VERSION 2.8)

project(AQQueryResolver)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.48.0 COMPONENTS program_options thread filesystem system timer regex REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  $ENV{AQUTIL_SOURCE_DIR}
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  $ENV{AQUTIL_LIBRARY_DIR}
)

# Mac OS X specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(OperatingSystem "Darwin")
    set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11 -stdlib=libc++ -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++")
    set(CMAKE_SHARED_LINKER_FLAGS "-lc++")
    set(CMAKE_MODULE_LINKER_FLAGS "-lc++")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

file(GLOB_RECURSE aq_query_resolver_source_files aq/*.cpp)
file(GLOB aq_engine_source_files aq/AQEngine.cpp aq/AQMatrix.cpp)
list(REMOVE_ITEM aq_query_resolver_source_files ${aq_engine_source_files})

if(EXISTS "aq/parser/lex.yy.cpp")
  file(GLOB exclude_files "aq/parser/lex.yy.cpp")
  list(REMOVE_ITEM aq_query_resolver_source_files ${exclude_files})
endif()

# Create target for the parser
ADD_CUSTOM_TARGET(SQL_PARSER echo "Creating sql parser")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
  SOURCE aq/parser/SQL92_Tok.l
  COMMAND flex 
  ARGS --nounput -oaq/parser/lex.yy.cpp aq/parser/SQL92_Tok.l
  TARGET SQL_PARSER
  OUTPUTS aq/parser/lex.yy.cpp
)

# Create custom command for bison/yacc (note the DEPENDS)
ADD_CUSTOM_COMMAND(
  SOURCE aq/parser/SQL92_Grm.y
  COMMAND bison 
  ARGS -y aq/parser/SQL92_Grm.y -o aq/parser/sql92_grm_tab.cpp
  TARGET SQL_PARSER
  DEPENDS aq/parser/lex.yy.cpp
  OUTPUTS aq/parser/sql92_grm_tab.cpp
)

SET(aq_query_resolver_source_files ${aq_query_resolver_source_files} aq/parser/sql92_grm_tab.cpp)
SET_SOURCE_FILES_PROPERTIES(aq/parser/lex.yy.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(aq/parser/sql92_grm_tab.cpp GENERATED)

# aq-engine static library
add_library(aq-engine STATIC ${aq_engine_source_files})

# aq-engine shared library
add_library(aq-engine-shared SHARED ${aq_engine_source_files})
target_link_libraries(aq-engine-shared aq-util ${Boost_LIBRARIES})
set_target_properties(aq-engine-shared PROPERTIES OUTPUT_NAME "aq-engine")

# aq-query-resolver static library
add_library(aq-query-resolver STATIC ${aq_query_resolver_source_files})

# aq-query-resolver shared library
add_library(aq-query-resolver-shared SHARED ${aq_query_resolver_source_files})
target_link_libraries(aq-query-resolver-shared aq-engine aq-util ${Boost_LIBRARIES})
set_target_properties(aq-query-resolver-shared PROPERTIES OUTPUT_NAME "aq-query-resolver")
