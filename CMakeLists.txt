project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option (DEPLOY_TEST "Test deployment" OFF)

include_directories( ${CMAKE_SOURCE_DIR} )

add_library(bdb pool.cpp)


add_executable(bdb_put ${CMAKE_SOURCE_DIR}/tests/bdb_put.cpp)
target_link_libraries(bdb_put bdb)

set(MY_TARGETS bdb)
set(HEADERS bdb.h)

if(DEPLOY_TEST)
	make_directory(${CMAKE_INSTALL_PREFIX}/pools)
	make_directory(${CMAKE_INSTALL_PREFIX}/transcations)
	set(DPFILES clean.sh)
	set(MY_TARGETS ${MY_TARGETS} bdb_put)	
endif()

install (TARGETS ${MY_TARGETS}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION .)

install (FILES ${DPFILES} DESTINATION .)
install (FILES ${HEADERS} DESTINATION include)
