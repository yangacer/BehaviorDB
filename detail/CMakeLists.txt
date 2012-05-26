
include ( ${PROJECT_SOURCE_DIR}/rpath.cmake )

option(BDB_SHARED_LIB "Build shared library" OFF)

if(MINGW)
  add_definitions (-D__MINGW__)
endif()

#set (Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" )
#find_package ( Boost 1.45.0 )

#if(Boost_FOUND)
#  include_directories(${Boost_INCLUDE_DIRS})
#endif()

if(BDB_SHARED_LIB)
  set(LIB_TYPE SHARED)
  add_definitions (-DBDB_MAKE_DLL)
else()
  set(LIB_TYPE STATIC)
  add_definitions (-DBDB_STATIC)
endif()

include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/bdb  )

add_library( bdb ${LIB_TYPE}
  common.cpp chunk.cpp 
  v_iovec.cpp id_pool.cpp poolImpl.cpp 
  addr_iter.cpp bdbImpl.cpp 
  error.cpp bdb.cpp stat.cpp
  fixedPool.cpp array.cpp
  nt_bdbImpl.cpp
  hash_map.cpp)

target_link_libraries ( bdb ${Boost_LIBRARIES} )

install (TARGETS bdb DESTINATION lib EXPORT bdb-targets )
install (EXPORT bdb-targets DESTINATION lib)
