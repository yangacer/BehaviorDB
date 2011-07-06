#ifndef _BDB_STAT_HPP
#define _BDB_STAT_HPP

#include "common.hpp"

template <typename T>
struct IDPool;

template <typename T, typename U>
struct IDValPool;

namespace BDB {

	struct BDBImpl;
	struct pool;

	struct bdbStater 
	{
		bdbStater(MemStat *ms);

		unsigned long long 
		operator()(BDBImpl const* bdb) const;
		
		unsigned long long 
		operator()(pool const *pool) const;
		
		unsigned long long 
		operator()( IDPool<AddrType> const *idp) const;

		unsigned long long 
		operator()( IDValPool<AddrType, AddrType> const *idvp) const;

		MemStat *ms;
	};

} // end of namespace BDB

#endif // end of header
