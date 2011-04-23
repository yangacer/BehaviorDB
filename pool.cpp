
#include <cstdio>
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>
#include <algorithm>

#include "bdb.h"
#include "chunk.h"
#include "idPool.h"
//#include "refHistory.h"
//#include "GAISUtils/profiler.h"

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

#define MIGBUF_SIZ 2*1024*1024

// path delimeter macro
#ifdef _WINDOWS
#define PATH_DELIM "\\"
#else
#define PATH_DELIM "/"
#endif

//! \brief Pool - A Chunk Manager
struct Pool
{
	friend struct BehaviorDB;
	friend struct AddrIterator;

	Pool();
	~Pool();

	/** Create chunk file
	 *  @param chunk_size
	 *  @param conf
	 *  @remark Error Number: none.
	 *  @remark Any failure happend in this method causes
	 *  system termination.
	 */
	void 
	create_chunk_file(SizeType chunk_size, Config const &conf);
	
	/** Enable/disable logging of pool
	 *  @param do_log
	 *  @remark BehaviorDB enable pool log by default.
	 */
	void
	log(bool do_log);

	/** Get chunk size
	 *  @return Chunk size of this pool.
	 */
	SizeType 
	chunk_size() const;

	/** Put data to some chunk
	 *  @param data Data to be put into this pool.
	 *  @param size Size of the data.
	 *  @return Address for accesing data just put.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW.
	 */
	AddrType 
	put(char const* data, SizeType size);
	
	/** Overwrite a chunk
	 *  @param address
	 *  @param data
	 *  @param size
	 *  @return Address
	 *  @remark Error Number: SYSTEM_ERROR, NON_EXIST
	 */
	AddrType
	overwrite(AddrType address, char const *data, SizeType size);

	/** Append data to a chunk
	 *  @param address Indicate which chunk to be appended.
	 *  @param data Data to be appended.
	 *  @param size Size of the data.
	 *  @param next_pool_idx Next pool index estimated by BehaviorDB.
	 *  @param next_pool Pointer to next pool refered by next_pool_idx.
	 *  @return Address for accessing the chunk that stores concatenated data.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG.
	 */
	AddrType 
	append(AddrType address, char const* data, SizeType size, 
		AddrType next_pool_idx, Pool* next_pool);
	
	/** Get chunk
	 *  @param output Output buffer for placing retrieved data.
	 *  @param size Size of output buffer
	 *  @param address Indicate which chunk to be retrieved.
	 *  @return Size of the output buffer.
	 *  @remark Error Number: SYSTEM_ERROR, DATA_TOO_BIG.
	 *  @remark In order to enhance security of library. 
	 *  Client has to be responsible for ensuring the size of output buffer
	 *  being large enough.
	 */
	SizeType 
	get(char *output, SizeType const size, AddrType address);
	
	SizeType
	get(char *output, SizeType const size, AddrType address, StreamState *stream);
	
	/** Delete chunk
	 *  @param address Address of chunk to be deleted.
	 *  @return Address of chunk just deleted.
	 *  @remark Error Number: None.
	 */
	AddrType
	del(AddrType address);

	/** Error number.
	 *  Not zero when any applicaton or system error happened.
	 */
	int error_num;

protected:
	
	/** Seek to chunk header
	 *  @param address
	 *  @remark Error Number: SYSTEM_ERROR
	 */
	void 
	seekToHeader(AddrType address);


	/** Move data from one chunk to another pool
	 *  @param src_file File stream of another pool which had seeked to source data.
	 *  @param orig_size Size of original data.
	 *  @param data Data to be appended to the original data.
	 *  @param size Size of appended data.
	 *  @return Address of the chunk that stores concatenated data.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW
	 */
	AddrType
	migrate(std::fstream &src_file, ChunkHeader ch, 
		char const *data, SizeType size); 

	void
	write_log(char const *operation, 
		AddrType const* address,
		std::streampos tell,
		SizeType size = 0,
		char const *desc = 0,
		int src_line = 0);

	void clear_error();

private:
	Pool(Pool const &cp);
	Pool& operator=(Pool const &cp);
	
	Config conf_;
	SizeType chunk_size_;
	bool doLog_;
	std::fstream file_;
	IDPool<AddrType> idPool_;
	std::ofstream wrtLog_;
	char file_buf_[1024*1024];
	static char migbuf_[MIGBUF_SIZ];
	bool onStreaming_;

	int lock_;
	static refHistory rh_;
	static BehaviorDB::MigPredictorCB pred_;
};


unsigned int
MonitorPred(refHistory const& rh, AddrType address, unsigned int next_pool_idx)
{
	countResult cr = rh.count(address);
	printf("@\nPUT: %.3f\nAPPEND: %.3f\nGET: %.3f\n",
		100 * cr.count[0] / (double)rh.size(),
		100 * cr.count[1] / (double)rh.size(),
		100 * cr.count[2] / (double)rh.size()
	);

	return next_pool_idx;
}

// ---------------- Misc Functions --------------

char Pool::migbuf_[MIGBUF_SIZ]= {0};
refHistory Pool::rh_(10000);
BehaviorDB::MigPredictorCB Pool::pred_(0);

struct error_num_to_str
{
	char const *operator()(int error_num)
	{
		return buf[error_num];
	}
private:
	static char buf[6][40];
};

char error_num_to_str::buf[6][40] = {
	"No error",
	"Address overflow",
	"System error",
	"Data too big",
	"Pool locked",
	"Non exist address"
};

// ------------- BehaviorDB impl -----------------

error_num_to_str ETOS;

using std::setw;
using std::hex;
using std::setfill;
using std::endl;
using std::ios;

void BehaviorDB::init_()
{
#ifdef _WINDOWS
	fopen_s(&lock_, "bdb.lock", "rw");
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
	accLog_->open(cvt.str().c_str(), ios::out | ios::app);
	if(!accLog_->is_open()){
		accLog_->open("access.log", ios::out | ios::trunc);
		if(!accLog_->is_open()){
			fprintf(stderr, "Open access.log failed; system msg - ");
			fprintf(stderr, strerror(errno));
		}
	}
	accLog_->rdbuf()->pubsetbuf(accBuf_, 1000000);

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
		char const* name(cvt.str().c_str());
		file_.open(name, ios::in | ios::out | ios::ate);
		if(!file_.is_open()){
			file_.open(name, ios::in | ios::out | ios::trunc);
			if(!file_.is_open()){
				fprintf(stderr, "Pools initial: %s\n", strerror(errno));
				exit(1);
			}
		}
		file_.rdbuf()->pubsetbuf(file_buf_, 1024*1024);
	}

	
	cvt<<".log";
	{
		char const *name(cvt.str().c_str());
		// init write log
		wrtLog_.open(name, ios::out | ios::app);
		if(!wrtLog_.is_open()){
			wrtLog_.open(name, ios_base::out | ios_base::trunc);
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
		write_log("appErr", &address, file_.tellg(), ch.size + size, "Read header error", __LINE__);
		return -1;
	}

	if(ch.size + size > chunk_size_ ){//|| ch.liveness == conf_.migrate_threshold){ // need to migration
		
		/*
		// migrate to larger pool early
		// when the chunk is appended 127 times
		if(next_pool_idx < 15 && ch.liveness == conf_.migrate_threshold)
			next_pool_idx++;

		if( ch.size + size > next_pool[next_pool_idx].chunk_size() ){ // no pool for migration
			write_log("appErr", &address, file_.tellg(), ch.size + size, "Exceed supported chunk size", __LINE__);
			error_num = DATA_TOO_BIG;
			return -1;	
		}
		*/

		// true migration
		//if(next_pool_idx != address >> 28){

		// erase old header
		file_.clear();
		file_.seekp(-8, ios::cur);
		//seekToHeader(address);
		file_.write("00000000", 8);
		file_.tellg(); // without this line, read will fail(why?)

		//Profiler.begin("Migration");
		// determine next pool idx according to refHistory
		if(pred_)
			next_pool_idx = pred_(rh_, address, next_pool_idx);
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
	ch.liveness++;
	
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
	ch_new.liveness = ch_new.liveness>>(chunk_size_>>conf_.chunk_unit);

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

