#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "error.hpp"
#include "idPool.hpp"
#include "addr_iter.hpp"
#include "stat.hpp"
#include "stream_state.hpp"
#include <cassert>
#include <stdexcept>

namespace BDB {

	BDBImpl::BDBImpl(Config const & conf)
	: pools_(0), err_log_(0), acc_log_(0), global_id_(0)
	{
		conf.validate();
		init_(conf); 
	}
	
	BDBImpl::~BDBImpl()
	{
		delete global_id_;

		if(acc_log_) fclose(acc_log_);
		if(err_log_) fclose(err_log_);

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

		// init logs
		char fname[256] = {};
		if(conf.log_dir){
			char const* log_dir = (*conf.log_dir) ? conf.log_dir : conf.root_dir;
			if(strlen(log_dir) > 256)
				throw std::length_error("length of pool_dir string is too long\n");

			sprintf(fname, "%serror.log", log_dir);
			if(0 == (err_log_ = fopen(fname, "ab")))
				throw std::runtime_error("create error log file failed\n");
		
			if(0 != setvbuf(err_log_, err_log_buf_, _IOLBF, 256))
				throw std::runtime_error("setvbuf to log file failed\n");
			
			sprintf(fname, "%saccess.log", log_dir);
			if(0 == (acc_log_ = fopen(fname, "ab")))
				throw std::runtime_error("create access log file failed\n");
		
			if(0 != setvbuf(acc_log_, acc_log_buf_, _IOLBF, 256))
				throw std::runtime_error("setvbuf to log file failed\n");
		}

		// init IDValPool
		sprintf(fname, "%sglobal_id.trans", conf.root_dir);
		global_id_ = new IDValPool(fname, conf.beg, conf.end);
	}
	
	AddrType
	BDBImpl::put(char const *data, size_t size)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		
		if(!global_id_->avail()){
			error(ADDRESS_OVERFLOW, __LINE__);
			return -1;
		}

		unsigned int dir = addrEval::directory(size);
		if((unsigned int)-1 == dir){
			error(DATA_TOO_BIG, __LINE__);
			return -1;
		}
		AddrType rt(0), loc_addr(0);
		while(dir < addrEval::dir_count()){
			loc_addr = pools_[dir].write(data, size);
			if(loc_addr != -1)	break;
			dir++;
		}
		
		if(-1 == loc_addr){
			error(dir-1);
			return -1;
		}

		rt = addrEval::global_addr(dir, loc_addr);
		rt = global_id_->Acquire(rt);
		
		global_id_->Commit(rt);
		
		fprintf(acc_log_, "%-12s\t%08x\n", "put", size);

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
			if((unsigned int)-1 == next_dir){
				error(DATA_TOO_BIG, __LINE__);
				return -1;
			}
			AddrType next_loc_addr;
			if(npos == off)
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
			global_id_->Commit(addr);
			fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "insert", size, addr, off);
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
		global_id_->Commit(addr);
		
		fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "insert", size, addr, off);

		return addr;
	}

	AddrType
	BDBImpl::preserve(size_t preserve_size, char const *data, size_t size)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		assert(preserve_size > size && 
			"preserve size should be greater than size");

		unsigned int dir = addrEval::directory(preserve_size);
		AddrType rt(0), loc_addr(0);
		while(dir < addrEval::dir_count()){
			loc_addr = pools_[dir].write(data, size);
			if(loc_addr != -1)	break;
			dir++;
		}
		
		if(-1 == loc_addr){
			error(dir-1);
			return -1;
		}

		rt = addrEval::global_addr(dir, loc_addr);
		rt = global_id_->Acquire(rt);

		if(-1 == rt){
			global_id_->Release(rt);
			error(ADDRESS_OVERFLOW, __LINE__);
			if(-1 == pools_[dir-1].free(loc_addr)){
				error(dir-1);
			}
			return -1;
		}
		
		global_id_->Commit(rt);
		fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "preserve", preserve_size, size);
		return rt;
	
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
		fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "update", size, addr);
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
		fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "get", size, addr, off);
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
		fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "string_get", max, addr, off);
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
		
		if(-1 == pools_[dir].free(loc_addr)){
			error(dir);
			return -1;	
		}
		global_id_->Release(addr);
		global_id_->Commit(addr);
		fprintf(acc_log_, "%-12s\t%08x\n", "del", addr);
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
		fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "partial_del", addr, off, size);
		return nsize;
	}
	
	bool
	BDBImpl::stream_error(stream_state const* state)
	{ return state->error; }

	stream_state const*
	BDBImpl::ostream(size_t stream_size)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");
		
		if(!global_id_->avail()){
			error(ADDRESS_OVERFLOW, __LINE__);
			return 0;
		}
		
		unsigned int dir = addrEval::directory(stream_size);
		AddrType inter_addr(0), loc_addr(0);
		while(dir < addrEval::dir_count()){
			loc_addr = pools_[dir].write((char const*)0, stream_size);
			if(loc_addr != -1)	break;
			dir++;
		}
		
		if(-1 == loc_addr){
			error(dir-1);
			return 0;
		}

		inter_addr = addrEval::global_addr(dir, loc_addr);

		fprintf(acc_log_, "%-12s\t%08x\n", "ostream", stream_size);
		
		stream_state *rt = new stream_state;
		rt->read_write = stream_state::WRT;
		rt->existed = false;
		rt->error = false;
		rt->inter_addr = inter_addr;
		rt->offset = 0;
		rt->size = stream_size;
		rt->used = 0;

		return rt;
	}

	stream_state const*
	BDBImpl::ostream(size_t stream_size, AddrType addr, size_t off)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		AddrType internal_addr;
		if( !global_id_->isAcquired(addr) )
			return 0;
		
		internal_addr = global_id_->Find(addr);

		unsigned int dir = addrEval::addr_to_dir(internal_addr);
		AddrType loc_addr = addrEval::local_addr(internal_addr);
		
		ChunkHeader header;
		if(-1 == pools_[dir].head(&header, loc_addr)){
			error(dir);
			return 0;
		}
		
		unsigned int next_dir = addrEval::directory(stream_size + header.size);

		if(-1 == next_dir){
			error(DATA_TOO_BIG, __LINE__);
			return 0;
		}
		
		off = (npos == off) ? header.size : off;

		AddrType next_loc_addr =
			pools_[dir].merge_copy( 
				0, stream_size, loc_addr, off,
				&pools_[next_dir], &header); 
		
		if(-1 == next_loc_addr){
			error(next_dir);
			return 0;
		}
		
		fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "ostream_inplace", stream_size, addr, off);

		stream_state* rt = new stream_state;
		rt->read_write = stream_state::WRT;
		rt->existed = true;
		rt->error = false;
		rt->ext_addr = addr;
		rt->inter_addr = addrEval::global_addr(
				next_dir, next_loc_addr);
		rt->offset = off;
		rt->size = stream_size;
		rt->used = 0;

		return rt;
	}
	
	stream_state const*
	BDBImpl::stream_write(stream_state const* state, char const* data, size_t size)
	{
		stream_state *ss = const_cast<stream_state*>(state);
		if(ss->size - ss->used < size){
			ss->error = true;
			return ss;
		}

		unsigned int dir = addrEval::addr_to_dir(ss->inter_addr);
		AddrType loc_addr = addrEval::local_addr(ss->inter_addr);

		if(size != pools_[dir].overwrite(data, size, loc_addr, ss->offset + ss->used)){
			error(dir);
			ss->error = true;
			return ss;
		}
		
		ss->used += size;
		// complete
		if(ss->used == ss->size){
			if(ss->existed){
				global_id_->Update(ss->ext_addr, ss->inter_addr);
			}else {
				if(-1 == (ss->ext_addr = global_id_->Acquire(ss->inter_addr))){
					ss->error = true;
					return ss;
				}	
			}	
			global_id_->Commit(ss->ext_addr);
			delete ss;
			return 0;
		}
		return ss;
	}
	
	
	void
	BDBImpl::stream_abort(stream_state const* state)
	{
		stream_state *ss = const_cast<stream_state*>(state);
		
		unsigned int dir = addrEval::addr_to_dir(ss->inter_addr);
		AddrType loc_addr = addrEval::local_addr(ss->inter_addr);
		
		if(-1 == pools_[dir].free(loc_addr))
			error(dir);	
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
	BDBImpl::stat(Stat *s) const
	{
		if(!s) return;
		bdbStater bstat(s);
		bstat(this);
	}
	
	void
	BDBImpl::error(int errcode, int line)
	{
		assert(0 != *this && "BDBImpl is not proper initiated");

		if(0 == err_log_) return;
		
		//lock
		
		if(0 == ftello(err_log_)){ // write column names
			fprintf(err_log_, "Pool ID\tLine\tMessage\n");
		}
		
		fprintf(acc_log_, "None    \t%d\t%s\n", line, error_num_to_str()(errcode));
		
		//unlock
	}

	void
	BDBImpl::error(unsigned int dir)
	{	
		assert(0 != *this && "BDBImpl is not proper initiated");

		if(0 == err_log_) return;

		std::pair<int, int> err = pools_[dir].get_error();
		
		if(err.first == 0) return;

		// TODO lock log
		
		if(0 == ftello(err_log_)){ // write column names
			fprintf(err_log_, "Pool_ID  Line Message\n");
		}

		while(1){
			fprintf(err_log_, "%08x %4d %s\n", dir, err.second, error_num_to_str()(err.first));
			err = pools_[dir].get_error();
			if(err.first == 0) break;
		}

		// TODO unlock
	}

} // end of namespace BDB
