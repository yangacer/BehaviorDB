#ifndef _BDB_STAT_HPP
#define _BDB_STAT_HPP

#include "common.hpp"

template <typename T>
class IDPool;

template <typename T, typename U>
class IDValPool;

namespace BDB {

	struct BDBImpl;
	struct pool;

	struct bdbStater 
	{
		bdbStater(Stat *s);

		void
		operator()(BDBImpl const* bdb) const;
		
		void
		operator()(pool const *pool) const;
		
		void
		operator()( IDPool<AddrType> const *idp) const;

		void
		operator()( IDValPool<AddrType, AddrType> const *idvp) const;

		Stat *s;
	};

} // end of namespace BDB

#endif // end of header
