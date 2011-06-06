#include "error.hpp"
#include "poolImpl.hpp"
#include "v_iovec.hpp"
#include "boost/variant/apply_visitor.hpp"

namespace BDB {
	
	pool::pool()
	: dirID(0), 
	  work_dir(), trans_dir(),
	  addrEval(0), file_(0), idPool_(), headerPool_()
	{}

	pool::pool(pool::config const &conf)
	: dirID(conf.dirID), 
	  work_dir(conf.work_dir), trans_dir(conf.trans_dir), 
	  addrEval(conf.addrEval), file_(0), idPool_(), headerPool_(conf.dirID, conf.header_dir)
	{
		if(0 == addrEval){
			fprintf(stderr, "addr_eval missing\n");
			exit(1);
		}

		// create pool file
		char fname[work_dir.size() + 8];
		sprintf(fname, "%s%04x.pool", work_dir.c_str(), dirID);
		if(0 == (file_ = fopen(fname, "r+b"))){
			if(0 == (file_ = fopen(fname, "w+b"))){
				fprintf(stderr, "create pool failed\n");
				exit(1);
			}	
		}
		if(0 != setvbuf(file_, 0, _IONBF, 0)){
			fprintf(stderr, "setvbuf to pool file failed\n");
			exit(1);
		}

		// init idPool
		sprintf(fname, "%s%04x.tran", trans_dir.c_str(), dirID);
		idPool_.replay_transaction(fname);
		idPool_.init_transaction(fname);
	}
	
	pool::~pool()
	{
		if(*this)
			fclose(file_);
	}
	
	pool::operator void const*() const
	{ 
		if(!file_ || !addrEval || !idPool_)
			return 0;
		return this;
	}

	AddrType
	pool::write(char const* data, size_t size)
	{
		if(!*this) return -1;

		if(!idPool_.avail()){
			// no space error
			on_error(ADDRESS_OVERFLOW, __LINE__);
			return -1;
		}

		if(!addrEval->capacity_test(dirID, size)){
			on_error(DATA_TOO_BIG, __LINE__);
			return -1;
		}

		AddrType loc_addr = idPool_.Acquire();
		ChunkHeader header;
		header.size = size;
		
		if(-1 == headerPool_.write(header, loc_addr)){
			// write failure
			idPool_.Release(loc_addr);
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
		
		if(-1 == seek(loc_addr)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		if(size != fwrite(data, 1, size, file_)){
			// write failure
			idPool_.Release(loc_addr);
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		return loc_addr;

	}
	
	// TODO
	// off=-1 represent an append write
	AddrType
	pool::write(char const* data, size_t size, AddrType addr, size_t off, ChunkHeader const* header)
	{
		if(!*this) return -1;

		if(!idPool_.isAcquired(addr)){
			// error: do not support until bitmap idpool available
			on_error(NON_EXIST, __LINE__);
			return -1;
		}
		

		ChunkHeader loc_header;
		if(header)
			loc_header = *header;
		else if( -1 == headerPool_.read(&loc_header, addr)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
		
		if(size + loc_header.size > addrEval->chunk_size_estimation(dirID)){
			on_error(DATA_TOO_BIG, __LINE__);
			return -1;
		}

		off = (-1 == off) ? loc_header.size : off;
		
		// data need to be moved can not larger than move buffer
		size_t moved = loc_header.size - off;
		loc_header.size += size;
		
		if(moved > MIGBUF_SIZ)
			return merge_move(data, size, addr, off, this, &loc_header); 

		// update header 
		if(-1 == headerPool_.write(loc_header, addr)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		if(-1 == seek(addr, off)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		// read data to be moved into mig_buf
		if(moved && moved != fread(mig_buf_, 1, moved, file_)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;

		}
		
		if(moved && -1 == seek(addr, off)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		// write new data
		if(size != fwrite(data, 1, size, file_)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		// write buffered  moved data
		if(moved && moved != fwrite(mig_buf_, 1, moved, file_)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		return addr;
	}
	
	AddrType
	pool::write(viov* vv, size_t len)
	{
		size_t total(0);
		for(size_t i=0; i<len; ++i){
			total += vv[i].size;		
		}
		
		if(total > addrEval->chunk_size_estimation(dirID)){
			on_error(DATA_TOO_BIG, __LINE__);
			return -1;
		}

		ChunkHeader header;
		header.size = total;
		

		AddrType loc_addr = idPool_.Acquire();
		
		if(-1 == headerPool_.write(header, loc_addr)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
		
		if( -1 == seek(loc_addr) ){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
		
		write_viov wv;
		wv.dest = file_;
		wv.dest_pos = addr_off2tell(loc_addr, 0);
		wv.buf = mig_buf_;
		wv.bsize = MIGBUF_SIZ;
		off_t loopOff(0);
		for(size_t i=0; i<len; ++i){
			wv.size = vv[i].size;
			if( vv[i].size && 
			    0 == boost::apply_visitor(wv, vv[i].data)){
				// error handle
				idPool_.Release(loc_addr);
				on_error(SYSTEM_ERROR, __LINE__);
				return -1;
			}
			wv.dest_pos += vv[i].size;
		}

		return loc_addr;
	}

	size_t
	pool::read(char* buffer, size_t size, AddrType addr, size_t off, ChunkHeader const* header)
	{
		if(!idPool_.isAcquired(addr)){
			on_error(NON_EXIST, __LINE__);
			return -1;
		}

		ChunkHeader loc_header;
		if(header)
			loc_header = *header;
		else if(-1 == headerPool_.read(&loc_header, addr)) {
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
		if(off > loc_header.size)
			return 0;

		if(-1 == seek(addr, off)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		if(size != fread(buffer, 1, size, file_)){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		return loc_header.size;

	}

	
	AddrType
	pool::merge_move(char const*data, size_t size, AddrType src_addr, size_t off, 
		pool *dest_pool, ChunkHeader const* header)
	{
		ChunkHeader loc_header;
		if(header)
			loc_header = *header;
		else if( -1 == headerPool_.read(&loc_header, src_addr) ){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}
	
		viov vv[3];
		file_src fs;
		AddrType loc_addr;
		if(0 == off){ // prepend
			vv[0].data = data;
			vv[0].size = size;
			fs.fp = file_;
			fs.off = addr_off2tell(src_addr, 0);
			vv[1].data = fs;
			vv[1].size = loc_header.size;
			loc_addr = dest_pool->write(vv, 2);
		}else if(loc_header.size == off){ // append
			fs.fp = file_;
			fs.off = addr_off2tell(src_addr, 0);
			vv[0].data = fs;
			vv[0].size = loc_header.size;
			vv[1].data = data;
			vv[1].size = size;
			loc_addr = dest_pool->write(vv, 2);
		}else{ // insert
			fs.fp = file_;
			fs.off = addr_off2tell(src_addr, 0);
			vv[0].data = fs;
			vv[0].size = off;
			vv[1].data = data;
			vv[1].size = size;
			fs.off += off;
			vv[2].data = fs;
			vv[2].size = loc_header.size - off;
			loc_addr = dest_pool->write(vv, 3);
		}

		if(-1 == loc_addr){
			on_error(SYSTEM_ERROR, __LINE__);
			return -1;
		}

		idPool_.Release(src_addr);
		return loc_addr;
	}

	size_t
	pool::erase(AddrType addr)
	{ 
		if(idPool_.isAcquired(addr))
			idPool_.Release(addr);
		else {
			on_error(NON_EXIST, __LINE__);
			return -1;
		}
		return 0;
	}

	size_t
	pool::erase(AddrType addr, size_t off, size_t size)
	{ 
		if(!idPool_.isAcquired(addr)){
			on_error(NON_EXIST, __LINE__);
			return -1;
		}

		ChunkHeader header;
		headerPool_.read(&header, addr);
		
		if(off > header.size) return header.size;
	

		// overflow check
		size = (off + size > off) ?  
			(off + size > header.size) ? header.size - off : size
			: header.size - off;

		size_t toRead = header.size - size;
		
		header.size -= size;

		headerPool_.write(header, addr);
		
		size_t readCnt, loopOff(0);
		
		while(toRead > 0){
			readCnt = (toRead > MIGBUF_SIZ) ? MIGBUF_SIZ : toRead;
			seek(addr, off + size + loopOff);
			if(readCnt != fread(mig_buf_, 1, readCnt, file_)){
				on_error(SYSTEM_ERROR, __LINE__);
				return -1;
			}
			seek(addr, off + loopOff);
			if(readCnt != fwrite(mig_buf_, 1, readCnt, file_)){
				on_error(SYSTEM_ERROR, __LINE__);
				return -1;
			}
			loopOff += readCnt;
			toRead -= readCnt;
		}
		
		return header.size;
	}
	

	off_t
	pool::seek(AddrType addr, size_t off)
	{
		off_t pos = addr;
		pos *= addrEval->chunk_size_estimation(dirID);
		pos += off;
		if(-1 == fseeko(file_, pos, SEEK_SET)){
			return -1;
		}
		return pos;
	}


	off_t
	pool::addr_off2tell(AddrType addr, size_t off) const
	{
		off_t pos = addr;
		pos *= addrEval->chunk_size_estimation(dirID);
		pos += off;
		return pos;
	}
	
	void
	pool::on_error(int errcode, int line)
	{
		// TODO: lock for mutli proc/thread
		err_.push_back(std::make_pair<int, int>(errcode, line));	
		// unlock
	}
	
	std::pair<int, int>
	pool::get_error(){
		std::pair<int, int> rt(0,0);
		if(!err_.empty()){
			rt = err_.front();
			err_.pop_front();
		}
		return rt;
		
	}

} // end of BDB namespace
