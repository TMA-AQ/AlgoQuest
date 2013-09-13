cmake_minimum_required(VERSION 2.8)

project(AQQueryResolver)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

find_package(Boost 1.50.0 COMPONENTS program_options thread filesystem system timer REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${AQUTIL_SOURCE_DIR}
  /home/thomas/Projects/AlgoQuest/AQUtil
)

link_directories(
  ${LIBRARY_OUTPUT_PATH}
  ${Boost_LIBRARY_DIRS}
  ${AQUTIL_LIBRARY_DIR}
  /home/thomas/Projects/AlgoQuest/AQUtil/lib
)

set(CMAKE_CXX_FLAGS "-O0 -ggdb -std=c++11")

file(
  GLOB_RECURSE
  source_files
  aq/*.cpp
  aq/parser/sql92_grm_tab.cpp
)

file(GLOB exclude_files "aq/parser/lex.yy.cpp")
list(REMOVE_ITEM source_files ${exclude_files})

# Create target for the parser
ADD_CUSTOM_TARGET(SQL_PARSER echo "Creating sql parser")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
  SOURCE aq/parser/SQL92_Tok.l
  COMMAND flex 
  ARGS -oaq/parser/lex.yy.cpp aq/parser/SQL92_Tok.l
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

# Add parser.c to the list of sources
SET(source_files ${source_files} aq/parser/sql92_grm_tab.cpp)

# Since parser.c does not exists yet when cmake is run, mark
# it as generated
SET_SOURCE_FILES_PROPERTIES(aq/parser/sql92_grm_tab.cpp GENERATED)

# Include binary directory to include lexer.c in parser.c
# INCLUDE_DIRECTORIES(${Foo_BINARY_DIR}/src)

add_library(
  aq_query_resolver
  STATIC
  ${source_files}
)
