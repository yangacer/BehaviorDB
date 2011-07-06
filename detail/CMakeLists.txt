
include_directories( ${PROJECT_SOURCE_DIR}/bdb /usr/local/include )

add_library( bdb 
	common.cpp chunk.cpp 
	v_iovec.cpp poolImpl.cpp 
	addr_iter.cpp bdbImpl.cpp 
	error.cpp bdb.cpp stat.cpp)

install (TARGETS bdb DESTINATION lib EXPORT bdb-targets )
install (EXPORT bdb-targets DESTINATION lib)
