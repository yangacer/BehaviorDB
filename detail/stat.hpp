#ifndef _BDB_STAT_HPP
#define _BDB_STAT_HPP

#include "common.hpp"


namespace BDB {

	class IDPool;
	class IDValPool;
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
		operator()( IDPool const *idp) const;

		void
		operator()( IDValPool const *idvp) const;

		Stat *s;
	};

} // end of namespace BDB

#endif // end of header
