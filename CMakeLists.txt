project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

include (CTest)
set (CTEST_PROJECT_NAME "BehaviorDB-Testing")

#option (INSTALL_TEST "Install test program" ON)

include_directories( ${PROJECT_SOURCE_DIR} /usr/local/include)

add_library(bdb common.cpp chunk.cpp v_iovec.cpp poolImpl.cpp addr_iter.cpp bdbImpl.cpp error.cpp bdb.cpp)

add_executable (bdb_test ${PROJECT_SOURCE_DIR}/tests/bdb.cpp)
target_link_libraries(bdb_test bdb)

add_test (basic_op bdb_test)

install (TARGETS bdb LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)

install (FILES bdb.hpp common.hpp addr_iter.hpp DESTINATION include/bdb)
