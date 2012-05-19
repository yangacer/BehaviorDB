#include "idPool.hpp"
#include "file_utils.hpp"
#include "error.hpp"
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <sstream>

namespace BDB { 
    
  IDPool::IDPool()
  : beg_(0), end_(0), file_(0), bm_(), lock_(), 
    full_alloc_(dynamic), max_used_(0)
  {}
  
  IDPool::IDPool(char const* tfile, 
        AddrType beg, 
        AddrType end, 
        IDPoolAlloc alloc_policy ) 
  : beg_(beg), end_(end), 
    file_(0), bm_(), lock_(), 
      full_alloc_(alloc_policy), max_used_(0)
  {
    assert( 0 != tfile );
    assert( beg_ <= end_ );
    assert((AddrType)-1 > end_);
    
    Bitmap::size_type size = end - beg;

    if(dynamic == full_alloc_){
      size >>= 16;
      bm_.resize(size, true);
      lock_.resize(size, false);
    }else if(full == full_alloc_){
      bm_.resize(size, true);
      lock_.resize(size, false);
    }

    replay_transaction(tfile);
    init_transaction(tfile);
  }
  
  IDPool::IDPool(AddrType beg, AddrType end)
  : beg_(beg), end_(end), file_(0), bm_(), lock_(), 
    full_alloc_(full), max_used_(0)
  {
    assert(end >= beg);

    bm_.resize(end_- beg_, true);
    lock_.resize(end_ - beg_, false);
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
    AddrType rt;
    // acquire priority: 
    // the one behind pos of (max_used() - 1)  >
    // the one behind pos of (max_used() - 1) after extending >
    // someone is located before pos
    rt = (max_used_) ? bm_.find_next(max_used_ - 1) : bm_.find_first() ;
    if((AddrType)Bitmap::npos == rt){
      try {
        extend();  
        rt = bm_.find_next(max_used_ - 1);
      }catch(addr_overflow const&){
        if(Bitmap::npos == (rt = bm_.find_first()))
          throw addr_overflow();
      }
    }
    bm_[rt] = false;
    if(rt >= max_used_) max_used_ = rt + 1;
    return  beg_ + rt;
  }
  
  AddrType
  IDPool::Acquire(AddrType const& id)
  {
    AddrType off = id - beg_;

    if(off >= bm_.size())
      extend(off+1); 
    bm_[off] = false;
    if(off >= max_used_) max_used_ = off + 1;
    return id;
  }
  
  int
  IDPool::Release(AddrType const &id)
  {
    assert(0 != this);
    assert(true == isAcquired(id) && "id is not acquired");
    
    AddrType off = id - beg_;
#ifndef NDEBUG
    if(off >= bm_.size() || lock_[off]) 
      throw invalid_addr();
#endif
    bm_[off] = true;
    return 0;
  }

  bool
  IDPool::Commit(AddrType const& id)
  {
    std::stringstream ss;
    char symbol = bm_[id-beg_] ? '-' : '+';
    ss<<symbol<<(id-beg_)<<"\n";

    return ss.str().size() == 
      detail::s_write(ss.str().c_str(), ss.str().size(), file_);
  }

  void
  IDPool::Lock(AddrType const &id)
  {
    lock_[id - beg_] = true;
  }

  void
  IDPool::Unlock(AddrType const &id)
  {
    lock_[id - beg_] = false;
  }
    
  bool
  IDPool::isLocked(AddrType const &id) const
  {
    return lock_[id - beg_];
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
        if(bm_.size() <= off)
          extend(off+1);
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
    
    if(0 != setvbuf(file_, filebuf_, _IOLBF, 128))
      throw std::runtime_error("IDPool: Fail to set zero buffer on transaction_file");
  }

  size_t 
  IDPool::num_blocks() const
  { return bm_.num_blocks(); }
  
  /// TODO Wrte testing case to generate exception
  void IDPool::extend(Bitmap::size_type new_size)
  { 
    Bitmap::size_type max = end_ - beg_;
    
    if(full_alloc_ == full || max == bm_.size() )
      throw addr_overflow();

    if(new_size){
      if(new_size > end_ - beg_)
        throw addr_overflow();
      try{
        bm_.resize(new_size, true);
        lock_.resize(new_size, false);
      }catch(std::bad_alloc const&){
        throw addr_overflow();         
      }
      return;
    }

    Bitmap::size_type size = bm_.size();
    new_size = (size<<1) -  (size>>1);
    
    if( new_size < size || new_size > max) 
      new_size = end_ - beg_;
    
    try{
      bm_.resize(size, true); 
      lock_.resize(size, false);
    }catch(std::bad_alloc const&){
      throw addr_overflow();
    }
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
  {  delete [] arr_;  }

  
  AddrType IDValPool::Acquire(AddrType const &val)//, error_code *ec)
  {
    AddrType rt;
    rt = super::Acquire();
    arr_[rt - super::begin()] = val;
    return rt;
    
  }
  
  AddrType IDValPool::Acquire(AddrType const& id, AddrType const& val)
  {
    AddrType rt  = super::Acquire(id);
    arr_[rt - super::begin()] = val;
    return rt;
  }

  bool IDValPool::avail() const
  {
    if(super::max_used() < super::end()) return true;
    return super::bm_.any(); 
  }

  bool 
  IDValPool::Commit(AddrType const& id)
  {
    AddrType off = id - begin();
    if(super::bm_[off]) 
      return super::Commit(id);

    std::stringstream ss;
    ss<<"+"<<(off)<<"\t"<<arr_[off]<<"\n";

    return ss.str().size() == 
      detail::s_write(ss.str().c_str(), ss.str().size(), file_);
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

