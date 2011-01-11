project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option (INSTALL_TEST "Install test program" ON)

include_directories( ${CMAKE_SOURCE_DIR} ~/GUtils/include)
link_directories( ~/GUtils/lib )

add_library(bdb pool.cpp chunk.cpp)


add_executable(bdb_put ${CMAKE_SOURCE_DIR}/tests/bdb_put.cpp)
target_link_libraries(bdb_put bdb GUtils)

add_executable(put ${CMAKE_SOURCE_DIR}/tests/put.cpp)
target_link_libraries(put bdb GUtils)

add_executable(chunk_test ${CMAKE_SOURCE_DIR}/tests/chunk_test.cpp chunk.cpp)

add_executable(append ${CMAKE_SOURCE_DIR}/tests/append.cpp)
target_link_libraries(append bdb GUtils)

add_executable(migrate ${CMAKE_SOURCE_DIR}/tests/migrate_test.cpp)
target_link_libraries(migrate bdb GUtils)

add_executable(getstream ${CMAKE_SOURCE_DIR}/tests/getstream.cpp)
target_link_libraries(getstream bdb GUtils)

add_executable(bench ${CMAKE_SOURCE_DIR}/tests/bench.cpp)
target_link_libraries(bench bdb GUtils)

set(MY_TARGETS bdb)
set(HEADERS bdb.h)


if(INSTALL_TEST)
	set(DPFILES clean.sh)	
	set(MY_TARGETS ${MY_TARGETS} bdb_put put append getstream migrate bench)
endif()

install (TARGETS ${MY_TARGETS}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION .)

install (FILES ${DPFILES} DESTINATION .)
install (FILES ${HEADERS} DESTINATION include)
