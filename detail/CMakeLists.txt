
include ( ${PROJECT_SOURCE_DIR}/rpath.cmake )

option(BDB_SHARED_LIB "Build shared library" OFF)

if(MINGW)
  add_definitions (-D__MINGW__)
endif()

if(BDB_SHARED_LIB)
  set(LIB_TYPE SHARED)
  add_definitions (-DBDB_MAKE_DLL)
else()
  set(LIB_TYPE STATIC)
  add_definitions (-DBDB_MAKE_STATIC)
endif()

include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/bdb  )

add_library( bdb ${LIB_TYPE}
  common.cpp chunk.cpp 
  v_iovec.cpp 
  id_pool.cpp id_handle.cpp
  poolImpl.cpp 
  addr_iter.cpp bdbImpl.cpp 
  error.cpp bdb.cpp stat.cpp
  fixedPool.cpp
  nt_bdbImpl.cpp)

target_link_libraries ( bdb ${Boost_LIBRARIES} )

install (TARGETS bdb DESTINATION lib EXPORT bdb-targets )
install (EXPORT bdb-targets DESTINATION lib)
