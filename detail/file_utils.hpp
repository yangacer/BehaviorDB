#ifndef BDB_FILE_UTILS_HPP_
#define BDB_FILE_UTILS_HPP_

#include "common.hpp"
#include <cstdio>
#include <cerrno>

#ifdef __MINGW__
#define ftello(X) ftello64(X)
#define fseeko(X,Y,Z) fseeko64(X,Y,Z)
#elif defined(_WIN32) || defined(_WIN64)
#define ftello(X) _ftelli64(X)
#define fseeko(X,Y,Z) _fseeki64(X,Y,Z)
#endif

namespace BDB {
namespace detail {
  
  inline uint32_t 
  s_write(char const* data, uint32_t size, FILE* fp)
  {
    uint32_t total_written = 0;

    while(size>0){
      errno = 0;
      uint32_t written = fwrite(data, 1, size, fp);
      total_written += written;
      if(written != size){
        if(errno == EINTR)
          continue;
        else if(errno != 0)
          return total_written;
      }
      data += written;
      size -= written;
    }
    return total_written;
  }
  
  
  inline uint32_t
  s_read(char *dest, uint32_t size, FILE* fp)
  {
    uint32_t total_read = 0;
    while(size > 0){
      errno = 0;
      uint32_t readcnt = fread(dest, 1, size, fp);
      total_read += readcnt;
      if(readcnt != size){
        if(errno == EINTR)
          continue;
        else if(errno !=0)
          return total_read;
      }
      dest += readcnt;
      size -= readcnt;
    }
    return total_read;
  }

  inline char 
  path_delim() 
  {
#if defined(_WIN32) || defined(_WIN64)
    return '\\';
#else
    return '/';
  }
#endif
#define PATH_DELIM detail::path_delim()

}} // namespace BDB::detail

#endif //header guard
