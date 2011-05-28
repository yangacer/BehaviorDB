
#include <cstdio>
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>
#include <algorithm>

#include "bdb.h"
#include "pool.hpp"
#include "chunk.h"
#include "utils.hpp"
//#include "refHistory.h"
//#include "GAISUtils/profiler.h"

#include <iostream>

// POSIX Headers
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

// path delimeter macro
#ifdef _WINDOWS
#define PATH_DELIM "\\"
#else
#define PATH_DELIM "/"
#endif


char Pool::migbuf_[MIGBUF_SIZ]= {0};
refHistory Pool::rh_(10000);


using std::setw;
using std::hex;
using std::setfill;
using std::endl;
using std::ios;


// ------------- AddrIterator Impl -------------

AddrIterator::AddrIterator()
: cur_(0), bdb_(0)
{}


AddrIterator::AddrIterator(BehaviorDB &bdb, AddrType cur)
: cur_(cur), bdb_(&bdb)
{}

AddrIterator::AddrIterator(AddrIterator const &cp)
: cur_(cp.cur_), bdb_(cp.bdb_)
{}

AddrIterator&
AddrIterator::operator=(AddrIterator const &cp)
{ cur_ = cp.cur_; bdb_ = cp.bdb_; return *this;}

AddrIterator&
AddrIterator::operator++()
{
	// endian check
	if(cur_ == -1){
		return *this;
	}

	bool reachEnd, acquired;
	int i = cur_>>28;
	AddrType iter = cur_ & 0x0fffffff;
	iter++;
	for(;i<16; ++i){
		while(	false == ( reachEnd = bdb_->pools_[i].idPool_.next() <= iter ) &&
			false == (acquired = bdb_->pools_[i].idPool_.isAcquired(iter)) )
			iter++;
		// wheather reach end of ith pool
		if(reachEnd){
			iter = 0;
			acquired = false;
		}
		if(acquired){
			cur_ = iter | i<<28;
			return *this;
		}
			
	}

	cur_ = -1;
	return *this;
}

AddrType
AddrIterator::operator*()
{
	if(!bdb_ || *this == bdb_->end())
		return -1;
	return cur_;	
}

bool
AddrIterator::operator==(AddrIterator const& rhs) const
{
	return cur_ == rhs.cur_ && bdb_ == rhs.bdb_;
}

// ------------- Pool implementation ------------

Pool::Pool()
: error_num(0), doLog_(true), idPool_(0, (1<<28)-1), onStreaming_(false)
{
	
}

Pool::~Pool()
{
	wrtLog_.close();
	file_.close();
}

void
Pool::create_chunk_file(SizeType chunk_size, Config const & conf)
{ 
	using namespace std;
	
	// setup by conf
	conf_ = conf;
	log(conf.pool_log);
	chunk_size_ = chunk_size;


	if(file_.is_open())
		file_.close();

	stringstream cvt;
	
	cvt<<conf_.working_dir<<PATH_DELIM<<"pools"<<PATH_DELIM
		<<setw(4)<<setfill('0')<<hex
		<< (chunk_size_>>conf_.chunk_unit)
		<< ".pool";
	{
		// create chunk file
		string name(cvt.str());
		file_.rdbuf()->pubsetbuf(file_buf_, 1024*1024);
		file_.open(name.c_str(), ios::in | ios::out | ios::ate | ios::binary);
		if(!file_.is_open()){
			file_.open(name.c_str(), ios::in | ios::out | ios::trunc | ios::binary);
			if(!file_.is_open()){
				fprintf(stderr, "Pools initial: %s\n", strerror(errno));
				exit(1);
			}
		}
		
	}

	
	cvt<<".log";
	{
		string name(cvt.str());
		// init write log
		wrtLog_.open(name.c_str(), ios::out | ios::app);
		if(!wrtLog_.is_open()){
			wrtLog_.open(name.c_str(), ios_base::out | ios_base::trunc);
			if(!file_.is_open()){
				fprintf(stderr, "Pool logs initial: %s\n", strerror(errno));
				exit(1);
			}
		}
		wrtLog_<<unitbuf;
	}

	cvt.clear();
	cvt.str("");
	cvt<<conf_.working_dir<<PATH_DELIM<<"transactions"<<PATH_DELIM
		<<setw(4)<<setfill('0')<<hex
		<< (chunk_size_>>conf_.chunk_unit)
		<<".trs";

	idPool_.replay_transaction(cvt.str().c_str());

	idPool_.init_transaction(cvt.str().c_str());

	return;
}

void
Pool::log(bool do_log)
{ doLog_ = do_log; }

SizeType
inline Pool::chunk_size() const
{ return chunk_size_; }

void
Pool::seekToHeader(AddrType address)
{
	std::streamoff off = (address & 0x0fffffff);
	file_.seekg(off * chunk_size_, ios::beg);
	file_.peek();
}



void
Pool::write_log(char const *operation, 
		AddrType const* address,
		std::streampos tell,
		SizeType size,
		char const *desc,
		int src_line)
{
	if(!doLog_) return;

	wrtLog_<<"["<<setw(10)<<setfill(' ')<<operation<<"]";
	
	if(address)
		wrtLog_<<" address: "<<setw(8)<<setfill('0')<<(*address&0x0fffffff);
	
	wrtLog_<<" tell(B): "<<setw(10)<<tell;

	if(size)
		wrtLog_<<" size(B): "<<setw(8)<<size;
	if(desc)
		wrtLog_<<" "<<desc;
	if(src_line)
		wrtLog_<<" src line: "<<src_line;

	wrtLog_<<endl;
}

void
Pool::clear_error()
{
	error_num &= ~(DATA_TOO_BIG | ADDRESS_OVERFLOW | POOL_LOCKED);	
}


AddrType
Pool::put(char const* data, SizeType size)
{
	
	clear_error();
	if(error_num)
		return -1;

	if(onStreaming_){
		error_num = POOL_LOCKED;
		write_log("putErr", 0, file_.tellp(), size, "Pool locked", __LINE__);
		return -1;
	}

	if(!idPool_.avail()){
		write_log("putErr", 0, file_.tellp(), size, "IDPool overflowed", __LINE__);
		error_num = ADDRESS_OVERFLOW;
		return -1; 
	}
	
	AddrType addr = idPool_.Acquire();
	std::streamoff off = addr;
	
	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	
	// write 8 bytes chunk header
	ChunkHeader ch;
	ch.size = size;
	file_<<ch;
	file_.write(data, (size));
	
	if(!file_.good()){
		idPool_.Release(addr);
		write_log("putErr", &addr, file_.tellp(), size, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	
	// write log
	write_log("put", &addr, file_.tellp(), size);
	
	return off;
}

AddrType
Pool::overwrite(AddrType address, char const* data, SizeType size)
{
	
	clear_error();
	if(error_num)
		return -1;

	if(onStreaming_){
		error_num = POOL_LOCKED;
		write_log("owrtErr", 0, file_.tellp(), size, "Pool locked", __LINE__);
		return -1;
	}

	if(!idPool_.isAcquired(address & 0x0fffffff)){
		write_log("owrtErr", 0, file_.tellp(), size, "IDPool overflowed", __LINE__);
		error_num = NON_EXIST;
		return -1; 
	}
	
	AddrType addr = address & 0x0fffffff;
	std::streamoff off = addr;
	
	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	
	// write 8 bytes chunk header
	ChunkHeader ch;
	ch.size = size;
	file_<<ch;
	file_.write(data, (size));
	
	if(!file_.good()){
		idPool_.Release(addr);
		write_log("owrtErr", &addr, file_.tellp(), size, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	
	// write log
	write_log("owrt", &addr, file_.tellp(), size);
	
	return off;
}

AddrType 
Pool::append(AddrType address, char const* data, SizeType size, 
	AddrType next_pool_idx, Pool* next_pool)
{
	using std::stringstream;
	
	//Profiler.begin("Pool Append");
	clear_error();
	if(error_num)
		return -1;
	
	if(onStreaming_){
		error_num = POOL_LOCKED;
		write_log("appErr", &address, file_.tellg(), size, "Pool locked", __LINE__);
		return -1;
	}
	
	ChunkHeader ch;
	
	// Test if the chunk is clean
	// Case 1: Acquired and released, then it is clean since we zero 
	//         out header of released chunk
	// Case 2: Not acquired, then use default ChunkHeader ch
	
	if(idPool_.cur_ > (address & 0x0fffffff)){
		//Profiler.begin("SeekHeader");
		seekToHeader(address);
		//Profiler.end("SeekHeader");
		//Profiler.begin("ReadHeader");
		file_>>ch;
		
		//Profiler.end("ReadHeader");
	}
	
	if(!file_){
		write_log("appErr", &address, file_.tellg(), ch.size + size, strerror(errno), __LINE__);
		return -1;
	}

	if(ch.size + size > chunk_size_ ){//|| ch.liveness == conf_.migrate_threshold){ // need to migration
		

		// true migration
		//if(next_pool_idx != address >> 28){

		// erase old header
		file_.clear();
		file_.seekp(-8, ios::cur);
		//seekToHeader(address);
		file_.write("00000000", 8);
		file_.tellg(); // without this line, read will fail(why?)
		file_.sync();

		//Profiler.begin("Migration");
		// determine next pool idx according to refHistory
		if(pred_)
			next_pool_idx = pred_(rh_, address, next_pool_idx);

		if(next_pool_idx == address>>28){
			write_log("appErr", &address, file_.tellg(), ch.size+size, "Data is larger than supported chuck", __LINE__);
			error_num = ADDRESS_OVERFLOW;
			return -1;	
		}

		AddrType rt = next_pool_idx<<28 | next_pool[next_pool_idx].migrate(file_, ch, data, size);
		//Profiler.end("Migration");
		if(-1 == rt && next_pool->error_num != 0){ // migration failed
			error_num = next_pool->error_num;
			return rt;
		}

		
		idPool_.Release(address & 0x0fffffff);
		//Profiler.end("Pool Append");
		return rt;
		//}
	}
	
	//Profiler.begin("Normal Append");
	// update header
	ch.size += size;
	// ch.liveness++;
	
	file_.clear();
	file_.seekp(-8, ios::cur);
	
	file_<<ch;
	
	if(!file_){
		write_log("appErr", &address, file_.tellp(), ch.size + size, "Write header error", __LINE__);
		return -1;
	}

	// append data
	file_.seekp(ch.size - size, ios::cur);
	file_.write(data, size);

	if(!file_){ // write failed
		write_log("appErr", &address, file_.tellp(), size, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	

	// write log
	write_log("append", &address, file_.tellg(), ch.size);
	//Profiler.end("Normal Append");
	//Profiler.end("Pool Append");
	return address;		
}

SizeType 
Pool::get(char *output, SizeType const size, AddrType address)
{
	clear_error();
	if(error_num)
		return -1;
	
	if(onStreaming_){
		error_num = POOL_LOCKED;
		write_log("getErr", &address, file_.tellg(), size, "Pool locked", __LINE__);
		return -1;
	}

	if(!idPool_.isAcquired(0x0fffffff & address)){
		return 0;
	}
	
	ChunkHeader ch;
	seekToHeader(address);
	file_>>ch;
	if(!file_){
		write_log("getErr", &address, file_.tellg(), ch.size + size, "Read header error", __LINE__);
		return -1;
	}


	if(ch.size > size){
		error_num = DATA_TOO_BIG;
		return ch.size;
	}
	
	file_.read(output, ch.size);

	if(ch.size != file_.gcount()){ // read failure
		write_log("getErr", &address, file_.tellg(), ch.size, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	// write log
	write_log("get", &address, file_.tellg(), ch.size);
	
	return size;

}

SizeType
Pool::get(char *output, SizeType const size, AddrType address, StreamState* stream)
{
	clear_error();
	if(error_num)
		return -1;
	
	if(!stream->left_){ // first read
		if(onStreaming_){
			error_num = POOL_LOCKED;
			write_log("getErr", &address, file_.tellg(), size, "Pool locked", __LINE__);
			return -1;
		}
		
		onStreaming_ = true;

		if(!idPool_.isAcquired(0x0fffffff & address)){
			return 0;
		}

		ChunkHeader ch;
		seekToHeader(address);
		file_>>ch;
		if(!file_){
			write_log("getErr", &address, file_.tellg(), ch.size + size, "Read header error", __LINE__);
			return -1;
		}

		stream->left_ = ch.size;
	}
	
	SizeType toRead = (size >= stream->left_) ? stream->left_ : size;
	
	file_.read(output, toRead);

	if(toRead != file_.gcount()){ // read failure
		write_log("getErr", &address, file_.tellg(), toRead, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	
	stream->left_ -= toRead;
	
	// write log
	write_log("get", &address, file_.tellg(), toRead);
	
	// Unlock this pool
	if(!stream->left_){
		onStreaming_ = false;	
	}

	return toRead;

}

AddrType
Pool::del(AddrType address)
{
	clear_error();
	if(error_num)
		return -1;

	idPool_.Release(address&0x0fffffff);

	// write log
	write_log("del", &address, file_.tellg());

	return address;
}

AddrType
Pool::migrate(std::fstream &src_file, ChunkHeader ch, 
	char const *data, SizeType size)
{
	clear_error();
	if(error_num)
		return -1;

	// !! For eliminating seek operations
	// assume the pptr() of src_file is located 
	// in head of original data
	
	ChunkHeader ch_new;
	ch_new.size = ch.size + size;
	// ch_new.liveness = ch_new.liveness>>(chunk_size_>>conf_.chunk_unit);

	if(!idPool_.avail()){
		write_log("migErr", 0, file_.tellp(), size, "IDPool overflowed", __LINE__);
		error_num = ADDRESS_OVERFLOW;
		return -1;
	}
	
	AddrType addr = idPool_.Acquire();
	std::streamoff off = addr;

	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	// write header
	file_<<ch_new;
	if(!file_){
		write_log("migErr", 0, file_.tellp(), size, "Write header error", __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	
	SizeType toRead = ch.size;

	while(toRead){
		(toRead >= MIGBUF_SIZ)?
			src_file.read(migbuf_, MIGBUF_SIZ) :
			src_file.read(migbuf_, toRead);
		if(!src_file.gcount()){
			write_log("migErr", &addr, src_file.tellg(), ch.size, strerror(errno), __LINE__);
			error_num = SYSTEM_ERROR;
			return -1;
		}
		file_.write(migbuf_, src_file.gcount());
		if(!file_){ // write failure
			write_log("migErr", &addr, file_.tellp(), ch.size, strerror(errno), __LINE__);
			error_num = SYSTEM_ERROR;
			return -1;
		}
		toRead -= src_file.gcount();
	}


	file_.write(data, size);
	if(!file_){ // write failure
		write_log("migErr", &addr, file_.tellp(), size, strerror(errno), __LINE__);
		error_num = SYSTEM_ERROR;
		return -1;
	}
	
	// write log
	write_log("migrate", &addr, file_.tellp(), ch_new.size);
	
	return addr;
}

