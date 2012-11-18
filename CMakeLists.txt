#Note the export.hpp.in depends on ${PROJECT_NAME}
project (BDB)
cmake_minimum_required(VERSION 2.8)

include (rpath.cmake)
include (use_c++11.cmake)

option( BDB_DEVEL_MODE OFF )

configure_file ( export.hpp.in ${CMAKE_SOURCE_DIR}/bdb/export.hpp)

set (Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" )
find_package ( Boost 1.45.0 COMPONENTS 
               serialization
               system)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_subdirectory( detail )

include_directories( ${PROJECT_SOURCE_DIR}/bdb ${PROJECT_SOURCE_DIR}/detail )

if(BDB_DEVEL_MODE)

  add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
  target_link_libraries(bdb_test bdb)

  add_executable (bdb_iter ${PROJECT_SOURCE_DIR}/tests/iterate.cpp)
  target_link_libraries (bdb_iter bdb)

  add_executable (sim ${PROJECT_SOURCE_DIR}/tools/simulator.cpp)
  target_link_libraries(sim bdb)

  add_executable (bdb_idp 
                  ${PROJECT_SOURCE_DIR}/tests/idpool.cpp
                  detail/fixedPool.cpp
                  detail/id_pool.cpp
                  detail/chunk.cpp)
  target_link_libraries (bdb_idp bdb)

  add_executable (bdb_iov
                  ${PROJECT_SOURCE_DIR}/tests/v_iovec.cpp
                  detail/v_iovec.cpp)
  target_link_libraries (bdb_iov bdb)
  
  add_executable (log tests/log.cpp)

endif()

add_executable (logcvt ${PROJECT_SOURCE_DIR}/tools/logcvt.cpp)

add_test (hashmap bdb_hashmap tmp/ )

#include (CTest)
#set (CTEST_PROJECT_NAME "BehaviorDB-Testing")
#add_test (basic_op bdb_test)

#install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
install (DIRECTORY bdb/ DESTINATION include/bdb)
