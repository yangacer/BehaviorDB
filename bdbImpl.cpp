#include "bdbImpl.hpp"

namespace BDB {

	BDBImpl::BDBImpl()
	: pools_(0), log_(0)
	{}

	BDBImpl::BDBImpl(Config const & conf)
	: pools_(0), log_(0)
	{ init_(conf); }
	
	BDBImpl::~BDBImpl()
	{
		for(unsigned int i =0; i<addrEval.dir_count(); ++i)
			pools_[i].~pool();
		free(pools_);
	}
	
	BDBImpl::operator void const*() const
	{
		return (0 != pool_) ? this : 0;
	}

	void
	BDBImpl::init_(Config const & conf)
	{
		addrEval.set(conf.min_size).set(conf.addr_prefix_len);

		// initial pools
		pool::config pcfg;
		pcfg.addrEval = &addrEval;
		pcfg.work_dir = conf.pool_dir;
		pcfg.trans_dir = conf.trans_dir;
		pcfg.header_dir = conf.header_dir;
		pools_ = (pool*)malloc(sizeof(pool) * addrEval.dir_count());
		for(unsigned int i =0; i<addrEval.dir_count(); ++i){
			pcfg.dir = i;
			new (&pools_[i]) pool(pcfg); 
		}

		// init log
		char fname[log_dir.size() + 9];
		sprintf(fname, "%serror.log", log_dir.c_str());
		if(0 == (log_ = fopen(fname, "r+b"))){
			if(0 == (log_ = fopen(fname, "w+b"))){
				fprintf(stderr, "create log file failed\n");
				exit(1);
			}	
		}
		if(0 != setvbuf(log_, 0, _IOLBF, 0)){
			fprintf(stderr, "setvbuf to log file failed\n");
			exit(1);
		}

	}
	
	AddrType
	BDBImpl::put(char const *data, size_t size)
	{
		// size to pool index
		unsigned int dir = addrEval.directory(size);
		AddrType rt(0);
		while(dir < addrEval.dir_count()){
			rt = pools_[dir].write(data, size);
			if(rt != -1){ 
				error(dir);
				break;
			}
			dir++;
		}
		return addrEval.global_addr(dir, rt);
	}


	AddrType
	BDBImpl::put(char const* data, size_t size, AddrType addr, size_t off)
	{
		unsigned int dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		ChunkHeader header;
		pools_[dir].head(&head, loc_addr);
		
		if(size + header.size > addEval.chunk_size_estimation(dir)){
			
			// migration
			unsigned int next_dir = (*MigPredictor)(addr);
			AddrType next_loc_addr;
			if(-1 == off)
				off = header.size;
			next_loc_addr = pools_[dir].merge_move( data, size, loc_addr, off,
					&pools_[next_dir], &header); 

			if(-1 == next_loc_addr){
				error(dir);
				error(next_dir);
				return -1;	
			}
			return addrEval.global_addr(next_dir, next_loc_addr);
		}

		// no migration
		// **Althought the chunk need not migrate to another pool, it might be moved to 
		// another chunk of the same pool due to size of data to be moved exceed size of 
		// migration buffer that a pool contains
		if(-1 == (loc_ addr = pools_[dir].write(data, size, loc_addr, off, &header)) ){
			error(dir);
			return -1;	
		}
		
		return addrEval.global_addr(dir, loc_addr);

	}

	size_t
	BDBImpl::get(char *output, size_t size, AddrType addr, size_t off)
	{
		size_t rt(0);
		unsigned int dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		if(-1 == (rt = pools_[dir].get(output, size, loc_addr, off))){
			error(dir);
			return -1;
		}
		return rt;
	}
	

	size_t
	BDBImpl::del(AddrType addr)
	{
		unsigned int dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);
		
		if(-1 == pools_[dir].erase(loc_addr)){
			error(dir);
			return -1;	
		}
		return addr;
	}

	size_t
	BDBImpl::del(AddrType addr, size_t off, size_t size)
	{
		unsigned int dir = addrEval.addr_to_dir(addr);
		AddrType loc_addr = addrEval.local_addr(addr);

		if(-1 == pools_[dir].erase(loc_addr, off, size)){
			error(dir);
			return -1;
		}
		return addr;
	}
	
	void
	BDBImpl::error(int errcode, int line)
	{
		if(0 == log_) return;
		
		//lock
		
		if(0 == ftello(log_)){ // write column names
			fprintf(log_, "Pool ID\tLine\tMessage\n");
		}
		
		fprintf(log_, "None    \t%d\t%s\n", dir, error.second, error_num_to_str()(error.first));
		
		//unlock
	}

	void
	BDBImpl::error(unsigned int dir)
	{	
		if(0 == log_) return;

		std::pair<int, int> error = pools_[dir].get_error();
		
		if(error.first == 0) return;

		// TODO lock log
		
		if(0 == ftello(log_)){ // write column names
			fprintf(log_, "Pool ID\tLine\tMessage\n");
		}

		while(1){
			fprintf(log_, "%08x\t%d\t%s\n", dir, error.second, error_num_to_str()(error.first));
			error = pools_[dir].get_error();
			if(error.first == 0) break;
		}

		// TODO unlock
	}

} // end of namespace BDB
