#include "bdbImpl.hpp"

namespace BDB {

	
	BDBImpl::BDBImpl():addrEval(4, 256){}
	BDBImpl::BDBImpl(Config const & conf){}
	BDBImpl::~BDBImpl(){}

	void
	BDBImpl::init_(Config const & conf)
	{}
	
	AddrType
	BDBImpl::put(char const *data, size_t size)
	{
		// size to pool index
		unsigned char dir = addrEval.directory(size);
		AddrType rt(0);
		while(dir < addrEval.dir_count()){
			rt = pools_[dir].write(data, size);
			if(rt != -1) 
				break;
			dir++;
		}
		return addrEval.global_addr(dir, rt);
	}

	AddrType
	BDBImpl::put(AddrType addr, char const *data, size_t size)
	{
		unsigned char dir = addrEval.addr_to_dir();
		AddrType loc_addr = addrEval.local_addr(addr);
		
		ChunkHeader header = pools_[dir].head(loc_addr);

		if(header.size + size > addrEval.chunk_size_estimation(dir) ){
			
			if(dir < addrEval.dir_count() - 1)
				return -1; // TODO error code or exception?
			
			// migration
			unsigned char next_dir = (*MigPredictor)(addr);
			AddrType next_loc_addr;
			if(-1 == (next_loc_addr = pools_[dir].move(loc_addr, &pools_[next_dir], &header)))
				return -1; // TODO
			
			if(-1 == pools_[nex_dir].write(next_loc_addr, data, size)){ 
				// TODO: how to notify intermediate result (data moved)?
			}
			return addrEval.global_addr(next_dir, next_loc_addr);
		}
		
		// no migration
		if(-1 == pools_[dir].write(loc_addr, data, size) ){
			// TODO
			return -1;	
		}

		return addr;
	}

	AddrType
	BDBImpl::put(AddrType addr, size_t off, char const* data, size_t size)
	{
		unsigned char dir = addrEval.addr_to_dir();
		AddrType loc_addr = addrEval.local_addr(addr);
		
		ChunkHeader header = pools_[dir].head(loc_addr, off);
		
		if(size + header.size > addEval.chunk_size_estimation(dir)){
			
			// migration
			unsigned char next_dir = (*MigPredictor)(addr);
			AddrType next_loc_addr;
			next_loc_addr = pools_[dir].merge_move(	loc_addr, off, data, size, 
								&pools_[next_dir], &header); 

			if(-1 == next_loc_addr){
				// TODO: error handle
				return -1;	
			}
			return addrEval.global_addr(next_dir, next_loc_addr);
		}

		// no migration
		if(-1 == pools_[dir].write(loc_addr, off, data, size, &header) ){
			// TODO: error handling
			return -1;	
		}
		
		return addr;

	}

	size_t
	BDBImpl::get(AddrType addr, char *output, size_t size)
	{}
	
	size_t
	BDBImpl::get(AddrType addr, size_t off, char *output, size_t size)
	{}

	AddrType
	BDBImpl::del(AddrType addr)
	{}

	AddrType
	BDBImpl::del(AddrType addr, size_t off, size_t size)
	{}

} // end of namespace BDB
