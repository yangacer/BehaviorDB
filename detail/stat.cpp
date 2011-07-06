#include "stat.hpp"
#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "idPool.hpp"
namespace BDB {
	
	bdbStater::bdbStater(MemStat *ms)
	: ms(ms)
	{}

	unsigned long long 
	bdbStater::operator()(BDBImpl const* bdb) const
	{
		unsigned long long rt(0);

		rt = ms->gid_mem_size = (*this)(bdb->global_id_);
		for(size_t i=0;i<addr_eval<AddrType>::dir_count();++i){
			ms->pool_mem_size += (*this)(bdb->pools_ + i);
		}
		rt += ms->pool_mem_size;
		return rt;
	}

	unsigned long long 
	bdbStater::operator()(pool const *pool) const
	{
		return (*this)(pool->idPool_) + MIGBUF_SIZ;
	}
	
	unsigned long long 
	bdbStater::operator()(IDPool<AddrType> const *idp) const
	{
		return sizeof(AddrType) * idp->num_blocks();		
	}

	unsigned long long 
	bdbStater::operator()(IDValPool<AddrType, AddrType> const *idvp) const
	{
		return 	sizeof(AddrType) * idvp->size() + 
			sizeof(AddrType) * idvp->num_blocks();
	}


} // end of namespace BDB
