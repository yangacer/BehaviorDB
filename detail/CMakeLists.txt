
<<<<<<< HEAD
option(BDB_BUILD_SHARED_LIB "Build shared library" OFF)
=======
option(BDB_SHARED_LIB "Build shared library" OFF)
>>>>>>> master

if(MINGW)
  add_definitions (-D__MINGW__)
endif()

<<<<<<< HEAD
if(BDB_BUILD_SHARED_LIB)
	set(LIB_TYPE SHARED)
	add_definitions (-DBDB_MAKE_DLL)
=======
set (Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" )
find_package ( Boost 1.45.0 )

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

if(BDB_SHARED_LIB)
  set(LIB_TYPE SHARED)
  add_definitions (-DBDB_MAKE_DLL)
>>>>>>> master
else()
  set(LIB_TYPE STATIC)
  add_definitions (-DBDB_STATIC)
endif()

<<<<<<< HEAD
include_directories( ${PROJECT_SOURCE_DIR} 
  ${PROJECT_SOURCE_DIR}/bdb /usr/local/include )

add_library( bdb ${LIB_TYPE}
	common.cpp chunk.cpp 
	v_iovec.cpp idPool.cpp poolImpl.cpp 
	addr_iter.cpp bdbImpl.cpp nt_bdbImpl.cpp
	error.cpp bdb.cpp array.cpp hash_map.cpp stat.cpp)
=======
include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/bdb  )

add_library( bdb ${LIB_TYPE}
  common.cpp chunk.cpp 
  v_iovec.cpp idPool.cpp poolImpl.cpp 
  addr_iter.cpp bdbImpl.cpp 
  error.cpp bdb.cpp stat.cpp
  addr_handle.cpp)
>>>>>>> master

install (TARGETS bdb DESTINATION lib EXPORT bdb-targets )
install (EXPORT bdb-targets DESTINATION lib)
