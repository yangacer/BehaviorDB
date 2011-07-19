#include "stat.hpp"
#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "idPool.hpp"
namespace BDB {
	
	bdbStater::bdbStater(Stat *s)
	: s(s)
	{}

	void
	bdbStater::operator()(BDBImpl const* bdb) const
	{

		(*this)(bdb->global_id_);
		
		for(size_t i=0;i<addr_eval<AddrType>::dir_count();++i){
			(*this)(bdb->pools_ + i);
		}
	}

	void
	bdbStater::operator()(pool const *pool) const
	{
		(*this)(pool->idPool_);

		s->disk_size += 
			pool->idPool_->max_used() * 
			pool::addrEval::chunk_size_estimation(pool->dirID);

		s->pool_mem_size += MIGBUF_SIZ;
	}
	
	void
	bdbStater::operator()(IDPool const *idp) const
	{
		s->pool_mem_size += sizeof(AddrType) * idp->num_blocks(); 
	}

	void
	bdbStater::operator()(IDValPool const *idvp) const
	{
		s->gid_mem_size += 
			sizeof(AddrType) * idvp->size() + 
			sizeof(AddrType) * idvp->num_blocks();
	}


} // end of namespace BDB
