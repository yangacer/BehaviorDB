
option(BUILD_SHARED_LIB "Build shared library" OFF)

if(BUILD_SHARED_LIB)
	set(LIB_TYPE SHARED)
	add_definitions ( -DBDB_MAKE_DLL )
else()
	set(LIB_TYPE STATIC)
	add_definitions ( -DBDB_STATIC )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/bdb /usr/local/include )

add_library( bdb ${LIB_TYPE}
	common.cpp chunk.cpp 
	v_iovec.cpp poolImpl.cpp 
	addr_iter.cpp bdbImpl.cpp 
	error.cpp bdb.cpp stat.cpp)

install (TARGETS bdb DESTINATION lib EXPORT bdb-targets )
install (EXPORT bdb-targets DESTINATION lib)
