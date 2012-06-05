#include "file_utils.hpp"
#include <stdexcept>

namespace BDB {
namespace detail{
  
  template<uint32_t RS>
  boost::pool<>
  s_buffer<RS>::pool_(RS);

  template<uint32_t RS>
  s_buffer<RS>::s_buffer()
  : buffer(0)
  {
    buffer = (char*)pool_.malloc();
    if(!buffer) throw std::bad_alloc();
  }

  template<uint32_t RS>
  s_buffer<RS>::~s_buffer()
  { pool_.free((void*)buffer);  }


  template<uint32_t RS>
  uint32_t s_buffer<RS>::size()
  { 
    return pool_.get_requested_size(); 
  }
  
  /*
  template<uint32_t RS>
  uint32_t s_buffer<RS>::alloc_size()
  {
    return pool_.get_max_size();
  }
  */

}} // namespace BDB::detail
