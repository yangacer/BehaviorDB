#include "error.hpp"
#include <cerrno>

namespace BDB {

  char error_num_to_str::buf[8][40] = {
    "No error",
    "Address overflow",
    "System error",
    "Data too big",
    "Pool locked",
    "Non exist address",
    "Rollback failure",
    "Commit Failure"
  };
  
  error_code::error_code()
  { errno = 0; }

} // end of namespace BDB
