#include "poolImpl.hpp"

namespace BDB {
	
	pool::pool()
	: dirID(0), work_dir(0), addrEval(0), file_(0)
	{}

	pool::pool(pool::config const &conf)
	: dirID(conf.dirID), work_dir(conf.work_dir), addrEval(conf.addrEval), file_(0)
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
		idPool_.init_transaction(fname);
	}
	
	pool::~pool()
	{
		if(*this)
			fclose(file_);
	}
	
	pool::operator void const*() const
	{ 
		if(0 == file_)
			return 0; 
		if(0 == addrEval)
			return 0;
		if(0 == idPool_)
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

		seek(loc_addr);
		if(-1 == write_header(file_, header)){
			// write failure
			idPool_.Release(loc_addr);
		}

		if(size != fwrite(data, 1, size, file_)){
			// write failure
			idPool_.Release(loc_addr);
		}

		return loc_addr;

	}
	
	// off=-1 represent an append write
	AddrType
	pool::write(char const* data, size_t size, AddrType addr, size_t off, ChunkHeader const* header)
	{
		if(!*this) return -1;

		if(!idPool_.isAcquired(addr) && off != -1){
			// error: do not support until bitmap idpool available
		}
		
		ChunkHeader loc_header;
		
		loc_header = (0==header) ? head(addr, 0) : *header ;
		off = (-1 == off) ? loc_header.size : off;
		
		// data need to be moved can not larger than move buffer
		size_t moved = loc_header.size - off;
		loc_header.size += size;
		
		seek(addr);
		if(-1 == write_header(file_, loc_header)){
			// TODO error	
		}

		if(moved != fread(mig_buf_, 1, moved, file_)){
			// TODO error 

		}
		
		seek(addr, off);
		
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
	{ return 0; }

	
	AddrType
	pool::merge_move(AddrType src_addr, size_t off, char const*data, size_t size,
		pool *dest_pool, ChunkHeader const* header)
	{
		// TODO seek is always required when multiple processes share this lib
		ChunkHeader loc_header = (0 == header) ? head(src_addr, off) : *header ;
		off = (-1 == off) ? loc_header.size : off;

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
		return dest_addr;
	}

	size_t
	pool::erase(AddrType addr)
	{ return 0; }

	size_t
	pool::erase(AddrType addr, size_t off, size_t size)
	{ return 0;}
	

	ChunkHeader
	pool::head(AddrType addr, size_t off)
	{
		off_t pos = addr;
		ChunkHeader header;

		pos *= addrEval->chunk_size_estimation(dirID);
		
		if(-1 == fseeko(file_, pos, SEEK_SET)){
			// TODO error handle	
		}

		if(-1 == read_header(file_, header)){
			// TODO error handle
		}

		if(off == -1) off = header.size;
		
		if(off > header.size){
			return header;	
		}
		
		pos += off + sizeof(ChunkHeader);
		if(-1 == fseeko(file_, pos, SEEK_SET)){
			// TODO error handle	
		}

		return header;
	}

	off_t
	pool::seek(AddrType addr, size_t off)
	{
		off_t pos = addr;
		pos *= addrEval->chunk_size_estimation(dirID);
		pos += off + sizeof(ChunkHeader);
		if(-1 == fseeko(file_, pos, SEEK_SET)){
			// TODO error handle	
		}
		return pos;
	}

} // end of BDB namespace
