#include "bdbImpl.hpp"

namespace BDB {

	BDBImpl::BDBImpl()
	: pools_(0)
	{}

	BDBImpl::BDBImpl(Config const & conf)
	{ init_(conf); }
	
	BDBImpl::~BDBImpl()
	{
		for(unsigned char i =0; i<addrEval.dir_count(); ++i)
			pools_[i].~pool();
		free(pools_);
	}
	
	BDBImpl::operator void*() const
	{ return static_cast<void *>(pools_); }

	void
	BDBImpl::init_(Config const & conf)
	{
		addrEval.set(conf.min_size).set(conf.addr_prefix_len);

		// TODO log issue

		// initial pools
		pool::config pcfg;
		pcfg.addrEval = &addrEval;
		pools_ = (pool*)malloc(sizeof(pool) * addrEval.dir_count());
		for(unsigned char i =0; i<addrEval.dir_count(); ++i){
			pcfg.dir = i;
			new (&pools_[i]) pool(pcfg); 
		}
	}
	
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
	BDBImpl::put(char const* data, size_t size, AddrType addr, size_t off)
	{
		unsigned char dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		ChunkHeader header = pools_[dir].head(loc_addr, off);
		
		if(size + header.size > addEval.chunk_size_estimation(dir)){
			
			// migration
			unsigned char next_dir = (*MigPredictor)(addr);
			AddrType next_loc_addr;
			if(-1 == off)
				off = header.size;
			next_loc_addr = pools_[dir].merge_move(	loc_addr, off, data, size, 
					&pools_[next_dir], &header); 

			if(-1 == next_loc_addr){
				// TODO: error handle
				return -1;	
			}
			return addrEval.global_addr(next_dir, next_loc_addr);
		}

		// no migration
		if(-1 == pools_[dir].write(data, size, loc_addr, off, &header) ){
			// TODO: error handling
			return -1;	
		}
		
		return addr;

	}

	size_t
	BDBImpl::get(char *output, size_t size, AddrType addr, size_t off)
	{
		size_t rt(0);
		unsigned char dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		if(-1 == (rt = pools_[dir].get(output, size, loc_addr, off))){
			// TODO: error handle
			return -1;
		}
		return rt;
	}
	

	AddrType
	BDBImpl::del(AddrType addr)
	{
		unsigned char dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		if(-1 == pools_[dir].erase(loc_addr)){
			// TODO: error handle
			return -1;	
		}
		return addr;
	}

	AddrType
	BDBImpl::del(AddrType addr, size_t off, size_t size)
	{
		unsigned char dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);

		if(-1 == pools_[dir].erase(loc_addr, off, size)){
			// TODO: err handle
			return -1;
		}
		return addr;
	}

} // end of namespace BDB
