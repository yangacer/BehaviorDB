#ifndef _STREAM_STATE_HPP
#define _STREAM_STATE_HPP

#include "common.hpp"

namespace BDB {
	struct stream_state 
	{
		bool read_write;	// 0 for read and 1 for wrt
		AddrType global_addr;
		AddrType loc_orig_addr;
		AddrType loc_new_addr;

		size_t offset; 	// offset from chunk begin
		size_t size;  	// size of stream
		size_t used; 	// read/written size
	};

} // end of namespace BDB

#endif // end of header
