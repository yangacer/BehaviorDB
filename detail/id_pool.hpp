#ifndef BDB_ID_POOL_HPP
#define BDB_ID_POOL_HPP

#include <boost/noncopyable.hpp>
#include <boost/dynamic_bitset.hpp>
#include <cstdio>
#include "common.hpp"

namespace BDB
{

enum IDPoolAlloc {
  dynamic = 0,
  full = 1
};


// TODO Intergrate IDPool and IDValPool into this
template<typename Array>
class IDPool 
: boost::noncopyable
{
  typedef typename Array::reference reference;
  typedef typename Array::value_type value_type;

  //IDPool(AddrType beg, AddrTeyp end);
  IDPool(char const* file, AddrType beg, AddrType end, IDPoolAlloc alloc_policy);
  ~IDPool();
  
  AddrType Acquire();
  AddrType Acquire(AddrType id);

  void Release(AddrType id);
  bool Commit(AddrType id, value_type const &val);

  void Lock(AddrType id);
  void Unlock(AddrType id);
  
  bool isAcquired(AddrType id) const;
  bool isLocked(AddrType id) const;

  reference Find(AddrType id);

  AddrType max_used() const;

  uint32_t size() const;
  uint32_t num_blocks() const;

  AddrType begin() const;
  AddrType end() const;

  void replay_transaction(char const* file);
  void init_transaction(char const* file);

private:
  
  void extend(uint32_t new_size);

  typedef boost::dynamic_bitset<uint32_t> Bitmap;


  AddrType const beg_, end_;
  Bitmap bm_;
  Bitmap lock_;
  IDPoolAlloc full_alloc_;
  AddrType max_used_;
  
  FILE* file_;

  Array arr_;
};
  
}

#endif //header guard
