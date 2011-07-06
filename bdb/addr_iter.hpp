#ifndef _ADDR_ITER_HPP
#define _ADDR_ITER_HPP

#include "export.hpp"
#include "common.hpp"

namespace BDB {

	struct BDBImpl;
	
	struct BDB_EXPORT AddrIterator 
	{
		friend struct BDBImpl;

		AddrIterator();
		AddrIterator(AddrIterator const &cp);
		
		AddrIterator& 
		operator=(AddrIterator const& cp);

		AddrIterator &
		operator++();
		
		/** Get the address on iterating
		 */
		AddrType
		operator*() const;
		
		bool 
		operator==(AddrIterator const& rhs) const;
		
		bool 
		operator!=(AddrIterator const& rhs) const
		{ return !(*this == rhs); }

	protected:
		AddrIterator(BDBImpl const &bdb, AddrType cur);

	private:
		AddrType cur_;
		BDBImpl const *bdb_;

	};
} // end of namespace BDB

#endif // end of header
