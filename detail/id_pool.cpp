#include "id_pool.hpp"

namespace BDB {

template<typename Array>
IDPool<Array>::IDPool(
  char const* file, 
  AddrType beg, AddrType end, 
  IDPoolAlloc alloc_policy)
{}

template<typename Array>
IDPool<Array>::~IDPool()
{}

template<typename Array>
AddrType IDPool<Array>::Acquire()
{}

template<typename Array>
AddrType IDPool<Array>::Acquire(AddrType id)
{}

template<typename Array>
void IDPool<Array>::Release(AddrType id)
{}

template<typename Array>
void IDPool<Array>::Commit(AddrType id)
{}

template<typename Array>
void IDPool<Array>::Lock(AddrType id)
{}

template<typename Array>
void IDPool<Array>::Unlock(AddrType id)
{}

template<typename Array>
bool IDPool<Array>::isAcquired(AddrType id) const
{}

template<typename Array>
bool IDPool<Array>::isLocked(AddrType id) const
{}

template<typename Array>
IDPool<Array>::Value IDPool<Array>::Find(AddrType id) id
{}

template<typename Array>
AddrType IDPool<Array>::max_used() const
{}

template<typename Array>
uint32_t IDPool<Array>::size() const
{}

template<typename Array>
uint32_t IDPool<Array>::num_blocks() const
{}

template<typename Array>
AddrType IDPool<Array>::begin() const
{}

template<typename Array>
AddrType IDPool<Array>::end() const
{}

template<typename Array>
void IDPool<Array>::replay_transaction(char const* file)
{}

template<typename Array>
void IDPool<Array>::init_transaction(char const* file)
{}

}// namespace BDB
