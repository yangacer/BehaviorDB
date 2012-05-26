#ifndef V_IOVEC_HPP
#define V_IOVEC_HPP

#include "boost/variant.hpp"
#include <cstdio>

namespace BDB {

  struct file_src
  {
    FILE *fp;
    off_t off;
  };
  
  struct blank_src
  {};

  struct viov
  {
    boost::variant<file_src, char const*, blank_src> data;
    size_t size;
  };

  size_t writevv(viov *vv, size_t len, FILE* dest, size_t off);

} // end of namespace BDB

#endif
