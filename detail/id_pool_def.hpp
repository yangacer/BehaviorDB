#ifndef BDB_IDPOOL_DEF_HPP_
#define BDB_IDPOOL_DEF_HPP_

#include "id_pool.hpp"
#include <sstream>
#include <fstream>
#include "error.hpp"
#include "file_utils.hpp"
#include "fixedPool.hpp"
#include "addr_wrapper.hpp"

namespace BDB{ 

template<typename Array>
IDPool<Array>::IDPool(
  unsigned int id, char const* work_dir,
  AddrType beg, AddrType end, 
  IDPoolAlloc alloc_policy)
: beg_(beg), end_(end), 
  bm_(), lock_(), 
  full_alloc_(alloc_policy), max_used_(0),
  file_(0),  arr_(0)
{
  if(beg >= end)
    throw std::invalid_argument(SRC_POS);

  arr_.template open(id, work_dir);
  Bitmap::size_type size = end - beg;

  if(dynamic == full_alloc_){
    while(size > 1024)
      size >>= 1;
  }
  bm_.resize(size, true);
  lock_.resize(size, false);
  arr_.template resize(size);

  char fname[40] = {};
  sprintf(fname, "%s%04x.tran", work_dir, id);
  replay_transaction(fname);
  init_transaction(fname);
}

template<typename Array>
IDPool<Array>::~IDPool()
{
  if(file_) fclose(file_);
}

template<typename Array>
AddrType IDPool<Array>::Acquire()
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
      if((AddrType)Bitmap::npos == (rt = bm_.find_first()))
        throw addr_overflow();
    }
  }
  bm_[rt] = false;
  if(rt >= max_used_) max_used_ = rt + 1;
  return  beg_ + rt;
}

template<typename Array>
AddrType IDPool<Array>::Acquire(AddrType id)
{
  AddrType off = id - beg_;

  if(off >= bm_.size())
    extend(off+1); 
  bm_[off] = false;
  if(off >= max_used_) max_used_ = off + 1;
  return id;
}

template<typename Array>
void IDPool<Array>::Release(AddrType id)
{
  assert(true == isAcquired(id) && "id is not acquired");
  AddrType off = id - beg_;
#ifndef NDEBUG
  if(off >= bm_.size())
    throw invalid_addr();
#endif
  if(lock_[off])
    return;
  bm_[off] = true;
}

template<typename Array>
bool IDPool<Array>::Commit(
  AddrType id,
  IDPool<Array>::value_type const &val)
{
  AddrType off = id - begin();
  std::stringstream ss;
  if(bm_[off])
    ss << '-' << off <<"\n";
  else{
    ss << '+' << off << "\t" << val << "\n";
    arr_.template store(val, off);
  }
  return 
    ss.str().size() == 
    detail::s_write(ss.str().c_str(), ss.str().size(), file_) &&
    0 == fflush(file_);
}

template<typename Array>
bool IDPool<Array>::ReleaseAndCommit(AddrType id)
{
  AddrType off = id - begin();
  std::stringstream ss;
  if(true == bm_[off])
    throw invalid_addr();
  bm_[off] = true;
  ss<<'-'<<off<<"\n";
  return 
    ss.str().size() == 
    detail::s_write(ss.str().c_str(), ss.str().size(), file_) &&
    0 == fflush(file_);
}

template<typename Array>
void IDPool<Array>::Lock(AddrType id)
{
  lock_[id - beg_] = false;
}

template<typename Array>
void IDPool<Array>::Unlock(AddrType id)
{
  lock_[id - beg_] = false;
}

template<typename Array>
bool IDPool<Array>::isAcquired(AddrType id) const
{
  Bitmap::size_type off = id - beg_;
  if(off >= bm_.size()) 
    return false;
  return bm_[off] == false;

}

template<typename Array>
bool IDPool<Array>::isLocked(AddrType id) const
{ return lock_[id - beg_];  }

template<typename Array>
typename IDPool<Array>::value_type 
IDPool<Array>::Find(AddrType id)
{ return arr_[id - beg_]; }

template<typename Array>
AddrType IDPool<Array>::max_used() const
{ return max_used_; }

template<typename Array>
AddrType IDPool<Array>::next_used(AddrType curID) const
{
  while(curID != bm_.size() + beg_){
    if(false == bm_[curID - beg_])
      return curID;
    ++curID;  
  }
  return end_;
}

template<typename Array>
typename IDPool<Array>::size_type IDPool<Array>::size() const
{ return bm_.size(); }

template<typename Array>
typename IDPool<Array>::size_type IDPool<Array>::num_blocks() const
{ return bm_.num_blocks();  }

template<typename Array>
bool IDPool<Array>::avail() const
{
  if(max_used() < end()) return true;
  return bm_.any();
}

template<typename Array>
AddrType IDPool<Array>::begin() const
{ return beg_; }

template<typename Array>
AddrType IDPool<Array>::end() const
{ return end_; }

template<typename Array>
void IDPool<Array>::replay_transaction(char const* file)
{
  using namespace std;

  assert(0 != file);
  assert(0 == file_ && "disallow replay when file_ has been initiated");

  ifstream tfile(file, ios::in | ios::binary);

  if(!tfile.is_open()) // no transaction files for replaying
    return;

  char op;
  AddrType off;
  value_type val;
  while(tfile >> op >> off){
    if('+' == op) {
      if('\t' == tfile.peek()) tfile.ignore(1);
      tfile >> val;
      if(bm_.size() <= off)
        extend(off+1);
      bm_[off] = false;
      arr_.template store(val, off);
      if(max_used_ <= off) max_used_ = off+1;
    }else if('-' == op){
      bm_[off] = true;
    }
  }
  tfile.close();
}

template<typename Array>
void IDPool<Array>::init_transaction(char const* file)
{
  assert(0 != file);
  if(0 == (file_ = fopen(file,"ab")))
    throw std::runtime_error("IDPool: Fail to open transaction file");

  if(0 != setvbuf(file_, 0, _IONBF, 0))
    throw std::runtime_error("IDPool: Fail to set zero buffer on transaction_file");
}

template<typename Array>
void IDPool<Array>::extend(uint32_t new_size)
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
      arr_.template resize(new_size);
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
    bm_.resize(new_size, true); 
    lock_.resize(new_size, false);
    arr_.template resize(new_size);
  }catch(std::bad_alloc const&){
    throw addr_overflow();
  }
}

} // namespace BDB

#endif
