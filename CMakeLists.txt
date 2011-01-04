project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option (INSTALL_TEST "Install test program" ON)

include_directories( ${CMAKE_SOURCE_DIR} )

add_library(bdb SHARED pool.cpp chunk.cpp)


add_executable(bdb_put ${CMAKE_SOURCE_DIR}/tests/bdb_put.cpp)
target_link_libraries(bdb_put bdb)

add_executable(largeput ${CMAKE_SOURCE_DIR}/tests/largeput.cpp)
target_link_libraries(largeput bdb)

add_executable(chunk_test ${CMAKE_SOURCE_DIR}/tests/chunk_test.cpp chunk.cpp)

add_executable(append ${CMAKE_SOURCE_DIR}/tests/append.cpp)
target_link_libraries(append bdb)

add_executable(getstream ${CMAKE_SOURCE_DIR}/tests/getstream.cpp)
target_link_libraries(getstream bdb)

set(MY_TARGETS bdb)
set(HEADERS bdb.h)


if(INSTALL_TEST)
	set(DPFILES clean.sh)	
	set(MY_TARGETS ${MY_TARGETS} bdb_put largeput append getstream)
endif()

install (TARGETS ${MY_TARGETS}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION .)

install (FILES ${DPFILES} DESTINATION .)
install (FILES ${HEADERS} DESTINATION include)
