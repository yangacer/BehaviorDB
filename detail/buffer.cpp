#include "buffer.hpp"
#include "bdbImpl.hpp"

namespace BDB {

	buffer::buffer(size_t buffer_size, buffer::open_mode mode, 
		BDBImpl const* bdb)
	: read_write(mode), 
	existed(false),
	error(false),
	ext_addr(),
	inter_addr(),
	offset(0),
	size(buffer_size),
	used(0),
	bdb_(const_cast<BDBImpl*>(bdb))
	{
		// AddrType bdb_->put(0, buffer_size);
	}

	buffer::buffer(size_t buffer_size, buffer::open_mode mode, 
		AddrType addr, size_t off, BDBImpl const* bdb)
	{}

} // end of namespace BDB
