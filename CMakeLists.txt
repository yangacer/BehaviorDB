project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

#include (CTest)
#set (CTEST_PROJECT_NAME "BehaviorDB-Testing")


add_subdirectory( detail )

include_directories( ${PROJECT_SOURCE_DIR}/bdb)

add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
target_link_libraries(bdb_test bdb)

add_test (basic_op bdb_test)

#install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
install (DIRECTORY bdb/ DESTINATION include/bdb)
