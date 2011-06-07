#include "bdb.h"
#include "pool.hpp"
#include "chunk.h"
#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// POSIX Headers
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef _WINDOWS
#include <sys/file.h>
#else
#include <direct.h>
#include <io.h>
#endif

// path delimeter macro
#ifdef _WINDOWS
#define PATH_DELIM "\\"
#else
#define PATH_DELIM "/"
#endif

using std::setw;
using std::hex;
using std::setfill;
using std::endl;
using std::ios;


error_num_to_str ETOS;

BehaviorDB::MigPredictorCB Pool::pred_(0);

void BehaviorDB::init_()
{
#ifdef _WINDOWS
	fopen_s(&lock_, "bdb.lock", "w");
	_lock_file(lock_);
#else
	lock_ = open("bdb.lock", O_RDWR);
	flock(lock_, LOCK_EX);
#endif
	

	if(1<<conf_.chunk_unit < sizeof(ChunkHeader)){
		fprintf(stderr, "Chunk unit cannot lower than 4\n");
		exit(1);
	}

	// init members other than conf_
	error_num = 0;
	pools_ = new Pool[16];
	accLog_ = new std::ofstream; 
	errLog_ = new std::ofstream;
	
	/// @todo TODO: Create directories (portability issue)
	std::stringstream cvt;
	cvt<<conf_.working_dir<<PATH_DELIM<<"transactions";

#ifdef _WINDOWS
	if(-1 == _mkdir(cvt.str().c_str()) && errno != EEXIST){
#else
	if(-1 == mkdir(cvt.str().c_str(), S_IRWXU | S_IRWXG) && errno != EEXIST){
#endif
		fprintf(stderr, "Create directory transactions failed - ");
		fprintf(stderr, strerror(errno));
		exit(1);
	}

	cvt.str("");
	cvt<<conf_.working_dir<<PATH_DELIM<<"pools";
#ifdef _WINDOWS
	if(-1 == _mkdir(cvt.str().c_str()) && errno != EEXIST){
#else
	if(-1 == mkdir(cvt.str().c_str(), S_IRWXU | S_IRWXG) && errno != EEXIST){
#endif
		fprintf(stderr, "Create directory pools failed - ");
		fprintf(stderr, strerror(errno));
		exit(1);
	}
	
	for(SizeType i=0;i<16;++i){
		pools_[i].create_chunk_file((1<<i)<<conf_.chunk_unit, conf_);	
	}
	
	cvt.str("");
	cvt.clear();
	cvt<<conf_.working_dir<<PATH_DELIM<<"access.log";

	// open access log
	accLog_->rdbuf()->pubsetbuf(accBuf_, 1000000);
	accLog_->open(cvt.str().c_str(), ios::out | ios::app);
	if(!accLog_->is_open()){
		accLog_->open("access.log", ios::out | ios::trunc);
		if(!accLog_->is_open()){
			fprintf(stderr, "Open access.log failed; system msg - ");
			fprintf(stderr, strerror(errno));
		}
	}
	

	cvt.str("");
	cvt<<conf_.working_dir;
	cvt<<PATH_DELIM<<"error.log";

	// open error log
	errLog_->open(cvt.str().c_str(), ios::out | ios::app);
	if(!errLog_->is_open()){
		errLog_->open("error.log", ios::out | ios::trunc);
		if(!errLog_->is_open()){
			fprintf(stderr, "Open error.log failed; system msg - ");
			fprintf(stderr, strerror(errno));
		}
	}
	*errLog_<<std::unitbuf;
}

BehaviorDB::BehaviorDB()
: conf_()
{
	init_();
}

BehaviorDB::BehaviorDB(Config const &conf)
: conf_(conf)
{
	init_();	
}

BehaviorDB::~BehaviorDB()
{
#ifdef _WINDOWS
	_unlock_file(lock_);
	fclose(lock_);
#else
	flock(lock_, LOCK_UN);
	close(lock_);
#endif
	errLog_->close();
	accLog_->close();
	delete accLog_;
	delete errLog_;
	delete [] pools_;
}

inline SizeType 
BehaviorDB::estimate_max_size(AddrType address)
{
	SizeType s = address >> 28;
	if(s > 15)
		return -1;
	return 1<<s;
}

inline AddrType 
BehaviorDB::estimate_pool_index(SizeType size)
{
	// Determin which pool to put
	AddrType pIdx(0);
	SizeType bound(size>>conf_.chunk_unit);
	
	while(bound > (1u<<pIdx))
		++pIdx;

	if(size > (1u<<pIdx)<<conf_.chunk_unit)
		++pIdx;


	return pIdx;
}

inline void
BehaviorDB::clear_error()
{
	// clear error bits except SYSTEM_ERROR
	error_num &= ~(ADDRESS_OVERFLOW | DATA_TOO_BIG | POOL_LOCKED);
}

bool
BehaviorDB::error_return()
{
	if(error_num != 0){
		log_access("error", 0, 0, "System error had not been recovered");
		return true;
	}
	return false;
}


void
BehaviorDB::log_access(char const *operation, AddrType address, SizeType size, char const *desc)
{
	static char buf[52];
	sprintf(buf, "[% 9s] Size(B):%8lu Address(HEX):%08x\n", operation, size, address);
	accLog_->write(buf, 51);
		
}

AddrType
BehaviorDB::put(char const* data, SizeType size)
{
	clear_error();
	if(error_return())	return -1;

	AddrType pIdx = estimate_pool_index(size+8);
	
	if(pIdx > 15){ // exceed capacity
		error_num = DATA_TOO_BIG;
		*errLog_<<"[error]"<<ETOS(error_num)<<size<<endl;
		return -1;
	}
	
	AddrType rt(0);
	// invoke put until a pool has available chunk
	do{
		rt = pools_[pIdx].put(data, size);
		pIdx++;
	}while(	rt == -1 && 
		ADDRESS_OVERFLOW == pools_[pIdx-1].error_num && 
		pIdx < 15);

	pIdx--;
	error_num = pools_[pIdx].error_num;

	if(rt == -1 && 0 != error_num){
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return -1;
	}

	pIdx = pIdx<<28 | rt;
	
	pools_[0].rh_.add(PUT, pIdx);

	// write access log
	log_access("put", pIdx, size);
	
	return pIdx;
}

AddrType
BehaviorDB::overwrite(AddrType address, char const* data, SizeType size)
{
	clear_error();
	if(error_return())	return -1;
	
	AddrType pIdx = estimate_pool_index(size+8);
	if(pIdx < address>>28){
		error_num = DATA_TOO_BIG;
		*errLog_<<"[error]"<<ETOS(error_num)<<size<<endl;
		return -1;
	}
	
	AddrType rt = pools_[pIdx].overwrite(address, data, size);
	
	error_num = pools_[pIdx].error_num;

	if(rt == -1 && 0 != error_num){
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return -1;
	}

	pIdx = pIdx<<28 | rt;
	
	pools_[0].rh_.add(PUT, pIdx);

	// write access log
	log_access("overwrite", pIdx, size);
	
	return pIdx;
}

struct wvCmp
{
	bool operator()(WriteVector const& x, WriteVector const &y)
	{
		return *x.address < *y.address;	
	}
};

int
BehaviorDB::append(WriteVector* wv, int wv_size)
{
	if(!wv) return 0;

	
	std::stable_sort(wv, wv+wv_size, wvCmp());
	AddrType rt;
	for(int i=0; i< wv_size; ++i){
		rt = append(*(wv[i].address), wv[i].buffer, wv[i].size);
		if(rt == -1 && error_num)
			return i;
		*wv[i].address = rt;
	}
	return wv_size;
}

AddrType
BehaviorDB::append(AddrType address, char const* data, SizeType size)
{
	clear_error();
	if(error_return())	return -1;
	
	AddrType pIdx = address >> 28;
	
	// check address roughly
	if(pIdx > 15){
		error_num = ADDRESS_OVERFLOW;
		*errLog_<<"[error]"<<ETOS(error_num)<<": "<<address<<endl;
		return -1;
	}

	AddrType rt, next_pIdx(0);
	
	// Estimate next pool ----------------
	// ! No preservation space for size value since
	// it had been counted as part of existed data
	next_pIdx = estimate_pool_index(size+((1<<pIdx)<<conf_.chunk_unit));

	if(next_pIdx > 15)
		next_pIdx = 15;

	rt = pools_[pIdx].append(address, data, size, next_pIdx, pools_);
	
	if(rt == -1 && pools_[pIdx].error_num != 0){
		error_num = pools_[pIdx].error_num;
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return -1;
	}
	
	if(address != rt)
		pools_[0].rh_.update(address, rt);
	pools_[0].rh_.add(APPEND, rt);
	
	// write access log
	log_access("append", rt, size);
	
	return rt;

}

SizeType
BehaviorDB::get(char *output, SizeType const size, AddrType address)
{
	clear_error();
	if(error_return())	return -1;
	
	AddrType pIdx = address >> 28;
	
	// check address roughly
	if(pIdx > 15){
		error_num = ADDRESS_OVERFLOW;
		*errLog_<<"[error]"<<ETOS(error_num)<<": "<<address<<endl;
		return -1;
	}
	

	SizeType rt = pools_[pIdx].get(output, size, address);
	
	if(rt == -1 || pools_[pIdx].error_num){
		error_num = pools_[pIdx].error_num;
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return rt;
	}
	
	pools_[0].rh_.add(GET, address);

	// write access log
	log_access("get", address, rt);

	return rt;
}

SizeType
BehaviorDB::get(char *output, SizeType size, AddrType address, StreamState *stream)
{
	clear_error();
	if(error_return()) return -1;
	
	if(stream->pool_ && !stream->left_)
		return 0;

	if(!stream->pool_){
		AddrType pIdx = address >> 28;
		// check address roughly
		if(pIdx > 15){
			error_num = ADDRESS_OVERFLOW;
			*errLog_<<"[error]"<<ETOS(error_num)<<": "<<address<<endl;
			return -1;
		}

		stream->pool_ = &pools_[pIdx];
	}

	SizeType rt = stream->pool_->get(output, size, address, stream);
	if(rt == -1 || stream->pool_->error_num){
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return rt;
	}
	
	if(stream->left_ == 0)
		pools_[0].rh_.add(GET, address);	

	log_access("get", address, rt);
	
	return rt;
	
}

void
BehaviorDB::stop_get(StreamState* stream)
{
	stream->pool_->onStreaming_ = false;
	stream->pool_ = 0;
	stream->left_ = 0;
}

AddrType
BehaviorDB::del(AddrType address)
{
	clear_error();
	if(error_return())	return -1;
	
	AddrType pIdx = address >> 28;
	
	// check address roughly
	if(pIdx > 15){
		error_num = ADDRESS_OVERFLOW;
		*errLog_<<"[error]"<<ETOS(error_num)<<": "<<address<<endl;
		return -1;
	}

	AddrType rt = pools_[pIdx].del(address);
	
	if(rt == -1 && pools_[pIdx].error_num){
		error_num = pools_[pIdx].error_num;
		*errLog_<<"[error]"<<ETOS(error_num)<<endl;
		return -1;
	}
	
	// warn client when free list too long
	if( pools_[pIdx].idPool_.freeListSize() > 1<<27 ){
		*errLog_<<"[warning]"<<" Pool "<<
			pIdx<<"'s freelist exceed half of pool size."<<endl;
	}
	
	pools_[0].rh_.remove(address);

	// write access log
	log_access("del", address, 0);

	return address;

}

BehaviorDB&
BehaviorDB::set_pool_log(bool do_log)
{
	for(int i=0;i<16;++i)
		pools_[i].log(do_log);

	return *this;	
}

AddrIterator 
BehaviorDB::begin()
{
	AddrType cur_(0);
	bool reachEnd, acquired;
	// find the first chunk
	int i=0;
	for(;i<16; ++i){
		while(	false == ( reachEnd = pools_[i].idPool_.next() <= cur_ ) &&
			false == (acquired = pools_[i].idPool_.isAcquired(cur_)) )
			cur_++;
		// wheather reach end of ith pool
		if(reachEnd){
			cur_ = 0;
			acquired = false;
		}
		if(acquired)
			return AddrIterator(*this, i<<28 | cur_);
	}
	
	return AddrIterator(*this, -1);
	

}

AddrIterator
BehaviorDB::end()
{
	return AddrIterator(*this, -1);	
}

void
BehaviorDB::register_mig_predictor(MigPredictorCB cbf)
{
	pools_[0].pred_ = cbf;
}


