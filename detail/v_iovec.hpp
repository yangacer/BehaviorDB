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
    uint32_t size;
  };

  uint32_t writevv(viov *vv, uint32_t len, FILE* dest, uint32_t off);

} // end of namespace BDB

#endif
