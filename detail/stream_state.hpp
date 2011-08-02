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
		AddrType inter_src_addr;
		AddrType inter_dest_addr;

		unsigned int offset; 	// offset from chunk begin
		unsigned int size;  	// size of stream
		unsigned int used; 	// read/written size
	};

} // end of namespace BDB

#endif // end of header
