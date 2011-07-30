#include "idPool.hpp"

#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <sstream>
//#include "boost/system/error_code.hpp"

namespace BDB { 

	int
	IDPool::write(char const* data, size_t size)
	{
		// using namespace boost::system;
		
		while(size>0){
			errno = 0;
			size_t written = fwrite(data, 1, size, file_);
			if(written != size){
				//ECType ec_tmp = error_code(errno, system_category());
				//if(errc::interrupted == ec_tmp) // EINTR
				if(errno == EINTR)
					continue;
				else if(errno != 0)
					return -1;
				//else if(errc::no_space_on_device == ec_tmp) // ENOSP
				//	*ec = make_error_code(bdb_errc::id_pool::disk_full);
				//else // EIO || EFBIG || EFAULT
				//	*ec = make_error_code(bdb_errc::id_pool::disk_failure);
				//return -1;
			}
			data += written;
			size -= written;
		}
		return 0;
	}

	
	IDPool::IDPool()
	: beg_(0), end_(0), file_(0), bm_(), full_alloc_(false), max_used_(0)
	{}

	
	IDPool::IDPool(char const* tfile, AddrType beg)
	: beg_(beg), end_(std::numeric_limits<AddrType>::max()-1), 
	file_(0), bm_(), full_alloc_(false), max_used_(0)
	{
		assert( 0 != tfile );
		assert( beg_ <= end_ );
		assert((AddrType)-1 > end_);

		bm_.resize(4096, true);
		
		replay_transaction(tfile);
		init_transaction(tfile);
	}

	
	IDPool::IDPool(char const* tfile, AddrType beg, AddrType end)
	: beg_(beg), end_(end), file_(0), bm_(), full_alloc_(true), max_used_(0)
	{
		assert(0 != tfile);
		assert( beg_ <= end_ );	
		assert((AddrType)-1 > end_);

		bm_.resize(end_- beg_, true);
		
		replay_transaction(tfile);
		init_transaction(tfile);
	}

	
	IDPool::IDPool(AddrType beg, AddrType end)
	: beg_(beg), end_(end), file_(0), bm_(), full_alloc_(true), max_used_(0)
	{
		assert(end >= beg);
		bm_.resize(end_- beg_, true);
	}

	
	IDPool::~IDPool()
	{ if(file_) fclose(file_); }

	
	IDPool::operator void const*() const
	{
		if(!this || !file_) return 0;
		return this;
	}

	
	bool 
	IDPool::isAcquired(AddrType const& id) const
	{ 
		if(id - beg_ >= bm_.size()) 
			return false;
		return bm_[id - beg_] == false;

	}

	
	AddrType
	IDPool::Acquire()
	{
		// using namespace boost::system;

		assert(0 != this);
		//assert(0 != ec);
		
		AddrType rt;
		
		// extend bitmap
		if((AddrType)Bitmap::npos == (rt = bm_.find_first())){
			if(!full_alloc_ ) {
				try {
					extend();  
				}catch(std::bad_alloc const& e){ 
					//*ec = make_error_code(bdb_errc::id_pool::bitmap_resize_failure);
					return -1;
				}
				rt = bm_.find_first();
			}else{
				//*ec = make_error_code(bdb_errc::id_pool::bitmap_full);
				return -1;	
			}
		}
		/*
		std::stringstream ss;
		ss<<"+"<<rt<<"\n";
		if(-1 == write(ss.str().c_str(), ss.str().size()))
			return -1;
		*/
		bm_[rt] = false;

		if(rt >= max_used_) max_used_ = rt + 1;

		return 	beg_ + rt;
	}
		
	
	int
	IDPool::Release(AddrType const &id)//, ECType *ec)
	{
		assert(0 != this);
		
		if(id - beg_ >= bm_.size())
			return -1;
		/*
		std::stringstream ss;
		ss<<"-"<<(id-beg_)<<"\n";
		if(-1 == write(ss.str().c_str(), ss.str().size()))
			return -1;
		*/
		bm_[id - beg_] = true;
		return 0;
	}

	int
	IDPool::Commit(AddrType const& id)
	{
		std::stringstream ss;
		char symbol = bm_[id-beg_] ? '-' : '+';
		ss<<symbol<<(id-beg_)<<"\n";
		return write(ss.str().c_str(), ss.str().size());
	}

	AddrType
	IDPool::next_used(AddrType curID) const
	{
		if(curID >= end_ ) return end_;
		while(curID != end_){
			if(false == bm_[curID - beg_])
				return curID;
			++curID;	
		}
		return curID;
	}

	
	AddrType
	IDPool::max_used() const
	{ return max_used_; }

	
	size_t
	IDPool::size() const
	{ return bm_.size(); }

	
	void 
	IDPool::replay_transaction(char const* transaction_file)
	{
		assert(0 != transaction_file);

		assert(0 == file_ && "disallow replay when file_ has been initiated");

		FILE *tfile = fopen(transaction_file, "rb");

		if(0 == tfile) // no transaction files for replaying
			return;

		char line[21] = {0};		
		AddrType off;
		while(fgets(line, 20, tfile)){
			line[strlen(line)-1] = 0;
			off = strtoul(&line[1], 0, 10);
			if('+' == line[0]){
				if(bm_.size() <= off){ 
					if(full_alloc_)
						throw std::runtime_error("ID in trans file does not fit into idPool");
					else	
						extend();
				}
				bm_[off] = false;
				if(max_used_ <= off) max_used_ = off+1;
			}else if('-' == line[0]){
				bm_[off] = true;
			}
		}
		fclose(tfile);
	}

	
	void 
	IDPool::init_transaction(char const* transaction_file)
	{
		
		assert(0 != transaction_file);

		if(0 == (file_ = fopen(transaction_file,"ab")))
			throw std::runtime_error("IDPool: Fail to open transaction file");
		
		
		if(0 != setvbuf(file_, (char*)0, _IONBF, 0))
			throw std::runtime_error("IDPool: Fail to set zero buffer on transaction_file");

	}


	
	size_t IDPool::num_blocks() const
	{ return bm_.num_blocks(); }

	
	void IDPool::extend()
	{ 
		Bitmap::size_type size = bm_.size();
		size = (size<<1) -  (size>>1);

		if( size < bm_.size() || size >= end_ - beg_)
			return;

		bm_.resize(size, true); 
	}

	// ------------ IDValPool Impl ----------------

	
	IDValPool::IDValPool(char const* tfile, AddrType beg, AddrType end)
	: super(beg, end), arr_(0)
	{
		arr_ = new AddrType[end - beg];
		if(!arr_) throw std::bad_alloc();

		replay_transaction(tfile);
		super::init_transaction(tfile);
	}

	
	IDValPool::~IDValPool()
	{
		delete [] arr_;	
	}

	
	AddrType IDValPool::Acquire(AddrType const &val)//, error_code *ec)
	{
		//using namespace boost::system;

		if(!*this) return -1;

		AddrType rt;
		
		if((AddrType)super::Bitmap::npos == (rt = super::bm_.find_first()) ){
			// *ec = make_error_code(bdb_errc::id_pool::bitmap_full);
			return -1;
		}
		/*
		std::stringstream ss;
		ss<<"+"<<rt<<"\t"<<val<<"\n";

		if(-1 == write(ss.str().c_str(), ss.str().size()))
			return -1;
		*/
		super::bm_[rt] = false;

		if(rt >= super::max_used_)
			super::max_used_ = 1 + rt;

		arr_[rt] = val;

		return 	super::beg_ + rt;
	}
	
	bool IDValPool::avail() const
	{ return super::bm_.any(); }

	int IDValPool::Commit(AddrType const& id)
	{
		AddrType off = id - begin();
		if(super::bm_[off]) 
			return super::Commit(id);

		std::stringstream ss;
		ss<<"+"<<(off)<<"\t"<<arr_[off]<<"\n";
		return write(ss.str().c_str(), ss.str().size());
	}
	
	AddrType IDValPool::Find(AddrType const & id) const
	{
		assert(true == super::isAcquired(id) && "IDValPool: Test isAcquired before Find!");
		return arr_[ id - super::beg_ ];
	}


	
	void IDValPool::Update(AddrType const& id, AddrType const& val)
	{
		assert(true == super::isAcquired(id) && "IDValPool: Test isAcquired before Update!");
		
		if(val == Find(id)) return;
		
		/*
		std::stringstream ss;
		ss<<"+"<<(id - super::beg_)<<"\t"<<val<<"\n";

		if(ss.str().size() != fwrite(ss.str().c_str(), 1, ss.str().size(), super::file_))
			throw std::runtime_error("IDValPool(Update): write transaction failure");
		*/
		arr_[id - super::beg_] = val;

	}

	
	void IDValPool::replay_transaction(char const* transaction_file)
	{
		assert(0 != transaction_file);
		assert(0 == super::file_ && "disallow replay when file_ has been initiated");

		FILE *tfile = fopen(transaction_file, "rb");

		if(0 == tfile) // no transaction files for replaying
			return;
		

		char line[21] = {0};		
		AddrType off; 
		AddrType val;
		std::stringstream cvt;
		while(fgets(line, 20, tfile)){
			line[strlen(line)-1] = 0;
			cvt.clear();
			cvt.str(line +1);
			cvt>>off;
			if('+' == line[0]){
				cvt>>val;
				if(super::bm_.size() <= off)
					throw std::runtime_error("IDValPool: ID in trans file does not fit into idPool");
				super::bm_[off] = false;
				arr_[off] = val;
				if(off >= super::max_used_)
					super::max_used_ = off+1;
			}else if('-' == line[0]){
				super::bm_[off] = true;
			}
			assert(0 != cvt && "IDValPool: Read id-val pair failed");
		}
		fclose(tfile);
		
	}

} // end of namespace BDB

