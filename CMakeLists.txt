project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option (INSTALL_TEST "Install test program" ON)

include_directories( ${PROJECT_SOURCE_DIR} /usr/local/include)

add_library(bdb pool.cpp chunk.cpp refHistory.cpp bdb.cpp utils.cpp)

add_executable (pool poolImpl.cpp ${PROJECT_SOURCE_DIR}/tests/pool.cpp chunk.cpp v_iovec.cpp)
add_executable (fixpool ${PROJECT_SOURCE_DIR}/tests/fixpool.cpp chunk.cpp)

set(TEST_TARGETS bdb_put put append migrate_test getstream bench iterate addreval)
foreach(tg ${TEST_TARGETS})
	add_executable (${tg} ${PROJECT_SOURCE_DIR}/tests/${tg}.cpp)
	target_link_libraries(${tg} bdb)
endforeach(tg)

set(MY_TARGETS bdb)
set(HEADERS bdb.h)


if(INSTALL_TEST)
	set(DPFILES clean.sh)	
	set(MY_TARGETS ${MY_TARGETS} ${TEST_TARGETS})
endif()

install (TARGETS ${MY_TARGETS}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION .)

install (FILES ${DPFILES} DESTINATION .)
install (FILES ${HEADERS} DESTINATION include)
