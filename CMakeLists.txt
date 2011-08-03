project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

add_subdirectory( detail )

include_directories( ${PROJECT_SOURCE_DIR}/bdb ${PROJECT_SOURCE_DIR}/detail)

add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
target_link_libraries(bdb_test bdb)

add_executable (bdb_put ${PROJECT_SOURCE_DIR}/tests/put.cpp)
target_link_libraries(bdb_put bdb)

add_executable (bdb_streaming ${PROJECT_SOURCE_DIR}/tests/streaming.cpp)
target_link_libraries(bdb_streaming bdb)

add_executable (bdb_simulator ${PROJECT_SOURCE_DIR}/tests/simulator.cpp)
target_link_libraries(bdb_simulator bdb)

#add_executable (addreval ${PROJECT_SOURCE_DIR}/tests/addreval.cpp)
#target_link_libraries(addreval bdb)

include (CTest)
set (CTEST_PROJECT_NAME "BehaviorDB-Testing")
add_test (basic_op bdb_test)

#install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
install (DIRECTORY bdb/ DESTINATION include/bdb)
