#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_

#include "common.hpp"

namespace BDB {

	struct BDBImpl;

	struct buffer 
	{
		friend struct BDBImpl;

		enum open_mode { READ=0, WRT };

		size_t 
		write(char const* data, size_t size);

		size_t
		read(char *output, size_t size);
		
		void abort();

		bool error() const;

	private:	
		buffer(size_t buffer_size, open_mode mode, BDBImpl const* bdb);
		
		buffer(size_t buffer_size, open_mode mode, AddrType addr, size_t off, BDBImpl const* bdb);

		int handle; // reserve for future usage

		open_mode read_write;	// 0 for read and 1 for wrt
		bool existed;
		bool error;
		AddrType ext_addr;
		AddrType inter_addr;

		size_t offset; 	// offset from chunk begin
		size_t size;  	// size of buffer
		size_t used; 	// read/written size

		BDBImpl * bdb_;
	};
} // end of namespace BDB

#endif // end of header
