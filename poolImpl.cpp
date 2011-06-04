#include "poolImpl.hpp"

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
		sprintf(fname, "%s%02x.pool", work_dir.c_str(), dirID);
		if(0 == (file_ = fopen(fname, "r+b"))){
			if(0 == (file_ = fopen(fname, "w+b"))){
				fprintf(stderr, "create pool failed\n");
				exit(1);
			}	
		}

		// init idPool
		sprintf(fname, "%s%02x.tran", work_dir.c_str(), dirID);
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
		}

		AddrType loc_addr = idPool_.Acquire();
		ChunkHeader header;
		header.size = size;

		if(-1 == headerPool_.write(header, loc_addr)){
			// write failure
			idPool_.Release(loc_addr);
		}
		
		seek(loc_addr);

		if(size != fwrite(data, 1, size, file_)){
			// write failure
			idPool_.Release(loc_addr);
		}

		return loc_addr;

	}
	
	// TODO
	// off=-1 represent an append write
	AddrType
	pool::write(char const* data, size_t size, AddrType addr, size_t off, ChunkHeader const* header)
	{
		if(!*this) return -1;

		if(!idPool_.isAcquired(addr) && off != -1){
			// error: do not support until bitmap idpool available
		}
		
		ChunkHeader loc_header;
		if(!header)
			loc_header = *header;
		else
			headerPool_.read(&loc_header, addr);

		off = (-1 == off) ? loc_header.size : off;
		
		// data need to be moved can not larger than move buffer
		size_t moved = loc_header.size - off;
		loc_header.size += size;
		
		if(moved > MIGBUF_SIZ)
			return merge_move(data, size, addr, off, this, &loc_header); 
		


		// update header 
		if(-1 == headerPool_.write(loc_header, addr)){
			// TODO error	
		}

		seek(addr);

		// read data to be moved into mig_buf
		if(moved != fread(mig_buf_, 1, moved, file_)){
			// TODO error 

		}
		
		// write data to be moved
		if(size != fwrite(data, 1, size, file_)){
			// TODO error	
		}
		
		if(moved != fwrite(mig_buf_, 1, moved, file_)){
			// TODO error	
		}

		return addr;
	}
	
	size_t
	pool::read(char* buffer, size_t size, AddrType addr, size_t off, ChunkHeader const* header)
	{
		ChunkHeader loc_header;
		if(!header)
			loc_header = *header;
		else
			headerPool_.read(&loc_header, addr);
		
	}

	
	AddrType
	pool::merge_move(char const*data, size_t size, AddrType src_addr, size_t off, 
		pool *dest_pool, ChunkHeader const* header)
	{
		// TODO Without lock mechansim, seek is always required 
		// when multiple processes share this lib
		ChunkHeader loc_header;
		if(!header)
			loc_header = *header;
		else
			headerPool_.read(&loc_header, src_addr);
		off = (-1 == off) ? loc_header.size : off;
		
		// seek(src_addr, sizeof(ChunkHeader));

		size_t toRead = off; 
		size_t readCnt;
		AddrType dest_addr = dest_pool->write(0,0);
		
		while(toRead){
			// TODO substitute fread with pool read
			readCnt = (toRead > MIGBUF_SIZ) ? MIGBUF_SIZ : toRead;
			if(readCnt != fread(mig_buf_, 1, readCnt, file_)){
				// err handle	
			}
			// simple but slow here
			// TODO error handle
			dest_addr = dest_pool->write(mig_buf_, readCnt, dest_addr);
			toRead -= readCnt;
		}
		// TODO error handle
		dest_pool->write(data, size, dest_addr);


		toRead = loc_header.size - off;
		while(toRead){
			// TODO substitute fread with pool read
			readCnt = (toRead > MIGBUF_SIZ) ? MIGBUF_SIZ : toRead;
			if(readCnt != fread(mig_buf_, 1, readCnt, file_)){
				// err handle	
			}
			// simple but slow here
			// TODO error handle
			dest_addr = dest_pool->write(mig_buf_, readCnt, dest_addr);
			toRead -= readCnt;
		}

		idPool_.Release(src_addr);

		return dest_addr;
	}

	size_t
	pool::erase(AddrType addr)
	{ return 0; }

	size_t
	pool::erase(AddrType addr, size_t off, size_t size)
	{ return 0;}
	

	off_t
	pool::seek(AddrType addr, size_t off)
	{
		off_t pos = addr;
		pos *= addrEval->chunk_size_estimation(dirID);
		pos += off;
		if(-1 == fseeko(file_, pos, SEEK_SET)){
			// TODO error handle	
		}
		return pos;
	}

} // end of BDB namespace
