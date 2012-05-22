project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

include (rpath.cmake)

option( BDB_DEVEL_MODE OFF )

set (Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" )
find_package ( Boost 1.45.0 COMPONENTS 
               serialization)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_subdirectory( detail )

include_directories( ${PROJECT_SOURCE_DIR}/bdb ${PROJECT_SOURCE_DIR}/detail )
#link_directories ( /usr/local/lib)

if(BDB_DEVEL_MODE)
  add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
  target_link_libraries(bdb_test bdb)

  add_executable (bdb_simulator ${PROJECT_SOURCE_DIR}/tools/simulator.cpp)
  target_link_libraries(bdb_simulator bdb)

  add_executable (bdb_hashmap ${PROJECT_SOURCE_DIR}/tests/hash_map.cpp)
  target_link_libraries(bdb_hashmap bdb ${Boost_LIBRARIES})

  add_executable (bdb_array ${PROJECT_SOURCE_DIR}/tests/array.cpp)
  target_link_libraries(bdb_array bdb)

  add_executable (bdb_typed_array ${PROJECT_SOURCE_DIR}/tests/typed_array.cpp)
  target_link_libraries(bdb_typed_array bdb ${Boost_LIBRARIES})

  add_executable (bdb_sql ${PROJECT_SOURCE_DIR}/tests/sql.cpp)
  
  add_executable (bdb_idp 
                  ${PROJECT_SOURCE_DIR}/tests/idpool.cpp
                  detail/fixedPool.cpp
                  detail/id_pool.cpp
                  detail/chunk.cpp)
endif()

add_executable (logcvt ${PROJECT_SOURCE_DIR}/tools/logcvt.cpp)

add_test (hashmap bdb_hashmap tmp/ )

#include (CTest)
#set (CTEST_PROJECT_NAME "BehaviorDB-Testing")
#add_test (basic_op bdb_test)

#install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
install (DIRECTORY bdb/ DESTINATION include/bdb)
