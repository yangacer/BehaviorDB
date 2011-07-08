#include "common.hpp"
#include "version.hpp"
#include <stdexcept>
#include <limits>

namespace BDB {
	
	const size_t npos(std::numeric_limits<size_t>::max());
	char const* VERSION("bdb-"BDB_VERSION_);

	void
	Config::validate() const
	{
		using namespace std;
		
		if(beg >= end)
			throw invalid_argument("Config: beg should be less than end");
		
		if(addr_prefix_len >= (sizeof(AddrType)<<3))
			throw invalid_argument("Config: addr_prefix_len should be less than 8*sizeof(AddrType)");
		
		// min size check is delayed to runtime
		
		// path check is delayed till fopen, here we check path delimiter only
		if(*root_dir && PATH_DELIM != root_dir[strlen(root_dir)-1])
			throw invalid_argument("Config: non-empty root_dir should be ended with a path delimiter");

		if(*pool_dir && PATH_DELIM != pool_dir[strlen(pool_dir)-1])
			throw invalid_argument("Config: non-empty pool_dir should be ended with a path delimiter");

		if(*trans_dir && PATH_DELIM != trans_dir[strlen(trans_dir)-1])
			throw invalid_argument("Config: non-empty trans_dir should be ended with a path delimiter");

		if(*header_dir && PATH_DELIM != header_dir[strlen(header_dir)-1])
			throw invalid_argument("Config: non-empty header_dir should be ended with a path delimiter");

		if(*log_dir && PATH_DELIM != log_dir[strlen(log_dir)-1])
			throw invalid_argument("Config: non-empty log_dir should be ended with a path delimiter");
		
		if( (*cse_func)(0, min_size) >= (*cse_func)(1, min_size) )
			throw invalid_argument("Config: chunk_size_est should maintain strict weak ordering of chunk size");
		
		bool match = false;
		for(size_t i =1; i< min_size; ++i){
			if( (*ct_func)( (*cse_func)(0, min_size), i ) ) match = true;
		}
		if(!match) throw invalid_argument("Config: capacity_test should hold be true for some data size");
	}
} // end of namespace BDB