#include "stat.hpp"
#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "id_pool.hpp"
namespace BDB {
  
  bdbStater::bdbStater(Stat *s)
  : s(s)
  {}

  void
  bdbStater::operator()(BDBImpl const* bdb) const
  {

    (*this)(bdb->global_id_);
    
    for(size_t i=0;i< bdb->addrEval.dir_count();++i){
      (*this)(bdb->pools_ + i);
    }
  }

  void
  bdbStater::operator()(pool const *pool) const
  {
    (*this)(pool->idpool_);

    s->disk_size += 
      pool->idpool_->max_used()* 
      pool->addrEval.chunk_size_estimation(pool->dirID);

    s->pool_mem_size += MIGBUF_SIZ * 2;
  }
  
  template<typename T>
  void
  bdbStater::operator()(IDPool<T> const *idp) const
  {
    s->pool_mem_size += sizeof(AddrType) * idp->num_blocks(); 
  }

} // end of namespace BDB
