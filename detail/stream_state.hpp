#ifndef _STREAM_STATE_HPP
#define _STREAM_STATE_HPP

#include "common.hpp"

namespace BDB {

	struct stream_state 
	{
		enum{ READ=0, WRT };

		bool read_write;	// 0 for read and 1 for wrt
		bool existed;
		bool error;
		AddrType ext_addr;
		AddrType inter_addr;

		size_t offset; 	// offset from chunk begin
		size_t size;  	// size of stream
		size_t used; 	// read/written size
	};

} // end of namespace BDB

#endif // end of header
