#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "error.hpp"
#include "idPool.hpp"
#include "addr_iter.hpp"
#include <cassert>

namespace BDB {

	BDBImpl::BDBImpl()
	: pools_(0), log_(0), global_id_(0)
	{}

	BDBImpl::BDBImpl(Config const & conf)
	: pools_(0), log_(0), global_id_(0)
	{
		conf.validate();
		init_(conf); 
	}
	
	BDBImpl::~BDBImpl()
	{
		delete global_id_;
		if(!pools_) return;
		for(unsigned int i =0; i<addrEval::dir_count(); ++i)
			pools_[i].~pool();
		free(pools_);
	}
	
	BDBImpl::operator void const*() const
	{
		if(!this || !addrEval::is_init() || !pools_)
			return 0;
		return this;
	}
	
	
	void
	BDBImpl::init_(Config const & conf)
	{
		addrEval::init(
			conf.addr_prefix_len, 
			conf.min_size, 
			conf.cse_func, 
			conf.ct_func);

		// initial pools
		pool::config pcfg;
		pcfg.work_dir = (*conf.pool_dir) ? conf.pool_dir : conf.root_dir;
		pcfg.trans_dir =(*conf.trans_dir) ?  conf.trans_dir : conf.root_dir;
		pcfg.header_dir = (*conf.header_dir) ? conf.header_dir : conf.root_dir;

		pools_ = (pool*)malloc(sizeof(pool) * addrEval::dir_count());
		for(unsigned int i =0; i<addrEval::dir_count(); ++i){
			pcfg.dirID = i;
			new (&pools_[i]) pool(pcfg); 
		}

		// init log
		char fname[256];
		if(conf.log_dir){
			char const* log_dir = (*conf.log_dir) ? conf.log_dir : conf.root_dir;
			if(strlen(log_dir) > 256){
				fprintf(stderr, "length of pool_dir string is too long\n");
				exit(1);
			}

			sprintf(fname, "%serror.log", log_dir);
			if(0 == (log_ = fopen(fname, "ab"))){
				fprintf(stderr, "create log file failed\n");
				exit(1);

			}
			if(0 != setvbuf(log_, log_buf_, _IOLBF, 256)){
				fprintf(stderr, "setvbuf to log file failed\n");
				exit(1);
			}
		}

		// init IDValPool
		global_id_ = new IDValPool<AddrType, AddrType>(conf.beg, conf.end);
		sprintf(fname, "%sglobal_id.trans", conf.root_dir);
		global_id_->replay_transaction(fname);
		global_id_->init_transaction(fname);
	}
	
	AddrType
	BDBImpl::put(char const *data, size_t size)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		unsigned int dir = addrEval::directory(size);
		AddrType rt(0);
		while(dir < addrEval::dir_count()){
			rt = pools_[dir].write(data, size);
			if(rt != -1){ 
				// error(dir);
				// return -1;
				break;
			}
			dir++;
		}
		
		if(-1 == rt){
			error(dir-1);
			return -1;
		}

		rt = addrEval::global_addr(dir, rt);
		rt = global_id_->Acquire(rt);

		if(-1 == rt){
			error(ADDRESS_OVERFLOW, __LINE__);
			return -1;
		}

		return rt;
	}


	AddrType
	BDBImpl::put(char const* data, size_t size, AddrType addr, size_t off)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		AddrType internal_addr;
		if( !global_id_->isAcquired(addr) )
			return -1;
		internal_addr = global_id_->Find(addr);

		unsigned int dir = addrEval::addr_to_dir(internal_addr);
		AddrType loc_addr = addrEval::local_addr(internal_addr);
		AddrType rt;

		ChunkHeader header;
		pools_[dir].head(&header, loc_addr);
		
		if( size + header.size > addrEval::chunk_size_estimation(dir)){
			
			// migration
			unsigned int next_dir = addrEval::directory(size + header.size); //(*MigPredictor)(addr);
			AddrType next_loc_addr;
			if(-1 == off)
				off = header.size;
			
			// TODO migrate failure 
			next_loc_addr = pools_[dir].merge_move( data, size, loc_addr, off,
					&pools_[next_dir], &header); 

			if(-1 == next_loc_addr){
				error(dir);
				error(next_dir);
				return -1;	
			}
			rt = addrEval::global_addr(next_dir, next_loc_addr);
			global_id_->Update(addr, rt);
			
			return addr;
		}

		// no migration
		// **Althought the chunk need not migrate to another pool, it might be moved to 
		// another chunk of the same pool due to size of data to be moved exceed size of 
		// migration buffer that a pool contains
		if(-1 == (loc_addr = pools_[dir].write(data, size, loc_addr, off, &header)) ){
			error(dir);
			return -1;	
		}
		
		rt = addrEval::global_addr(dir, loc_addr);
		global_id_->Update(addr, rt);
		
		return addr;


	}

	
	AddrType
	BDBImpl::update(char const *data, size_t size, AddrType addr)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		AddrType internal_addr;
		if( !global_id_->isAcquired(addr) )
			return -1;
		internal_addr = global_id_->Find(addr);

		unsigned int dir = addrEval::addr_to_dir(internal_addr);
		AddrType loc_addr = addrEval::local_addr(internal_addr);
		AddrType rt;

		if(-1 == (loc_addr = pools_[dir].replace(data, size, loc_addr)) ){
			error(dir);
			return -1;	
		}
		
		rt = addrEval::global_addr(dir, loc_addr);
		return addr;
	}

	size_t
	BDBImpl::get(char *output, size_t size, AddrType addr, size_t off)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		
		if( !global_id_->isAcquired(addr) )
			return 0;
		addr = global_id_->Find(addr);

		size_t rt(0);
		unsigned int dir = addrEval::addr_to_dir(addr);
		AddrType loc_addr = addrEval::local_addr(addr);
		
		if(-1 == (rt = pools_[dir].read(output, size, loc_addr, off))){
			error(dir);
			return 0;
		}
		return rt;
	}
	
	size_t
	BDBImpl::get(std::string *output, size_t max, AddrType addr, size_t off)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		
		if( !global_id_->isAcquired(addr) )
			return 0;
		addr = global_id_->Find(addr);


		size_t rt(0);
		unsigned int dir = addrEval::addr_to_dir(addr);
		AddrType loc_addr = addrEval::local_addr(addr);
		
		if( -1 == (rt = pools_[dir].read(output, max, loc_addr, off))){
			error(dir);
			return 0;
		}
		return rt;
	}

	size_t
	BDBImpl::del(AddrType addr)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
	
		AddrType internal_addr;
		
		if( !global_id_->isAcquired(addr) )
			return -1;
		internal_addr = global_id_->Find(addr);

		unsigned int dir = addrEval::addr_to_dir(internal_addr);
		AddrType loc_addr = addrEval::local_addr(internal_addr);
		
		if(-1 == pools_[dir].erase(loc_addr)){
			error(dir);
			return -1;	
		}
		global_id_->Release(addr);
		return 0;
	}

	size_t
	BDBImpl::del(AddrType addr, size_t off, size_t size)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		
	
		if( !global_id_->isAcquired(addr) )
			return -1;
		addr = global_id_->Find(addr);
	

		unsigned int dir = addrEval::addr_to_dir(addr);
		AddrType loc_addr = addrEval::local_addr(addr);
		size_t nsize;
		if(-1 == (nsize = pools_[dir].erase(loc_addr, off, size))){
			error(dir);
			return -1;
		}
		return nsize;
	}
	
	AddrIterator
	BDBImpl::begin() const
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		AddrType first_used = global_id_->begin();
		first_used = global_id_->next_used(first_used);
		
		return AddrIterator(*this, first_used);
	}

	AddrIterator
	BDBImpl::end() const
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		return AddrIterator(*this, global_id_->end());	
	}

	void
	BDBImpl::error(int errcode, int line)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		if(0 == log_) return;
		
		//lock
		
		if(0 == ftello(log_)){ // write column names
			fprintf(log_, "Pool ID\tLine\tMessage\n");
		}
		
		fprintf(log_, "None    \t%d\t%s\n", line, error_num_to_str()(errcode));
		
		//unlock
	}

	void
	BDBImpl::error(unsigned int dir)
	{	
		assert(0 != *this && "BDBImpl is not proper initiated");

		if(0 == log_) return;

		std::pair<int, int> err = pools_[dir].get_error();
		
		if(err.first == 0) return;

		// TODO lock log
		
		if(0 == ftello(log_)){ // write column names
			fprintf(log_, "Pool_ID  Line Message\n");
		}

		while(1){
			fprintf(log_, "%08x %4d %s\n", dir, err.second, error_num_to_str()(err.first));
			err = pools_[dir].get_error();
			if(err.first == 0) break;
		}

		// TODO unlock
	}

} // end of namespace BDB
