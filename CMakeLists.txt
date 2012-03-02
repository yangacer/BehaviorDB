project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option( DEVEL_MODE OFF )

add_subdirectory( detail )

include_directories( ${PROJECT_SOURCE_DIR}/bdb ${PROJECT_SOURCE_DIR}/detail /usr/local/include)
link_directories ( /usr/local/lib)

if(DEVEL_MODE)
  add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
  target_link_libraries(bdb_test bdb)

  add_executable (bdb_simulator ${PROJECT_SOURCE_DIR}/tools/simulator.cpp)
  target_link_libraries(bdb_simulator bdb)

  add_executable (bdb_hashmap ${PROJECT_SOURCE_DIR}/tests/hash_map.cpp)
  target_link_libraries(bdb_hashmap bdb boost_serialization)

  add_executable (bdb_array ${PROJECT_SOURCE_DIR}/tests/array.cpp)
  target_link_libraries(bdb_array bdb)

  add_executable (bdb_typed_array ${PROJECT_SOURCE_DIR}/tests/typed_array.cpp)
  target_link_libraries(bdb_typed_array bdb boost_serialization)
endif()

add_executable (logcvt ${PROJECT_SOURCE_DIR}/tools/logcvt.cpp)

include (CTest)
set (CTEST_PROJECT_NAME "BehaviorDB-Testing")
#add_test (basic_op bdb_test)
add_test (hashmap bdb_hashmap tmp/ )

#install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
install (DIRECTORY bdb/ DESTINATION include/bdb)
