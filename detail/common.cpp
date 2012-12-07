#include "common.hpp"
#include "file_utils.hpp"
#include "version.hpp"
#include <stdexcept>
#include <limits>
#include <cstring>

namespace BDB {
  
  const uint32_t npos(std::numeric_limits<uint32_t>::max());
  char const* VERSION("bdb-"BDB_VERSION_);
  
  Config::Config(
    AddrType beg,
    AddrType end,
    unsigned int addr_prefix_len,
    uint32_t min_size,
    char const *root_dir,
    char const *pool_dir,
    char const *trans_dir,
    char const *header_dir,
    char const *log_dir,
    Chunk_size_est cse_func,
    Capacity_test ct_func 
  )
  // initialization list
  : beg(beg), end(end),
  addr_prefix_len(addr_prefix_len), min_size(min_size), 
  root_dir(root_dir), pool_dir(pool_dir), 
  trans_dir(trans_dir), header_dir(header_dir), log_dir(log_dir),
  cse_func(cse_func), 
  ct_func(ct_func)
  { validate(); }

  void
  Config::validate() const
  {
    using namespace std;
    
    if(beg >= end)
      throw invalid_argument("Config: beg should be less than end");
    
    if(addr_prefix_len >= (sizeof(AddrType)<<3))
      throw invalid_argument("Config: addr_prefix_len should be less than 8*sizeof(AddrType)");
#define BDB_CHK_DIR_(DIR) \
    if(DIR.size() && PATH_DELIM != *(DIR.end()-1)) \
      throw invalid_argument( \
        "Config: non-empty " #DIR " should be ended with a path delimiter");

    // path check is delayed till fopen, here we check path delimiter only
    BDB_CHK_DIR_(root_dir);
    BDB_CHK_DIR_(pool_dir);
    BDB_CHK_DIR_(trans_dir);
    BDB_CHK_DIR_(header_dir);
    BDB_CHK_DIR_(log_dir);

    /*
    if(root_dir.size() && PATH_DELIM != root_dir.back())
      throw invalid_argument("Config: non-empty root_dir should be ended with a path delimiter");

    if(pool_dir.size() && PATH_DELIM != pool_dir.back())
      throw invalid_argument("Config: non-empty pool_dir should be ended with a path delimiter");

    if(PATH_DELIM != trans_dir.back())
      throw invalid_argument("Config: non-empty trans_dir should be ended with a path delimiter");

    if(PATH_DELIM != header_dir.back())
      throw invalid_argument("Config: non-empty header_dir should be ended with a path delimiter");

    if(PATH_DELIM != log_dir.back())
      throw invalid_argument("Config: non-empty log_dir should be ended with a path delimiter");
    */
    if( (*cse_func)(0, min_size) >= (*cse_func)(1, min_size) )
      throw invalid_argument("Config: chunk_size_est should maintain strict weak ordering of chunk size");
    
    bool match = false;
    for(uint32_t i =1; i< min_size; ++i){
      if( (*ct_func)( (*cse_func)(0, min_size), i ) ) match = true;
    }
    if(!match) throw invalid_argument("Config: capacity_test should hold be true for some data size");

    
  }
} // end of namespace BDB
