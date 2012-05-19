#ifndef BDB_ID_POOL_HPP
#define BDB_ID_POOL_HPP

#include <boost/noncopyable.hpp>
#include <boost/dynamic_bitset.hpp>
#include <fstream>
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
  typedef typename Array::value_type Value;
  
  //IDPool(AddrType beg, AddrTeyp end);
  IDPool(char const* file, AddrType beg, AddrType end, IDPoolAlloc alloc_policy);
  ~IDPool();
  
  AddrType Acquire();
  AddrType Acquire(AddrType id);

  void Release(AddrType id);
  void Commit(AddrType id);

  void Lock(AddrType id);
  void Unlock(AddrType id);
  
  bool isAcquired(AddrType id) const;
  bool isLocked(AddrType id) const;

  Value Find(AddrType id) id;

  AddrType max_used() const;

  uint32_t size() const;
  uint32_t num_blocks() const;

  AddrType begin() const;
  AddrType end() const;

  void replay_transaction(char const* file);
  void init_transaction(char const* file);

private:
  typedef boost::dynamic_bitset<uint32_t> Bitmap;

  void write(char const* data, size_t size);
  void extend(uint32_t size);

  AddrType const beg_, end_;
  Bitmap bm_;
  Bitmap lock_;
  IDPoolAlloc full_alloc_;
  AddrType max_used_;
  
  std::ofstream file_;

  Array arr_;
};
  
}

#endif //header guard
