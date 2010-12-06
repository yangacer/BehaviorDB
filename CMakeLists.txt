project (BehaviorDB)
cmake_minimum_required(VERSION 2.8)

option (DEPLOY_TEST "Test deployment" OFF)

add_executable(bdb pool.cpp)

if(DEPLOY_TEST)
	make_directory(${CMAKE_INSTALL_PREFIX}/pools)
	install (TARGETS bdb RUNTIME DESTINATION .)
endif()
