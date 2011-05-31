#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP
#include "addr_eval.hpp"

namespace BDB {
	
	// TODO 64bit opt
	typedef unsigned int AddrType;

	struct BDBImpl 
	{
		BDBImpl();
	        BDBImpl(Config const & conf);
		~BDBImpl();
		
		void
		init_(Config const & conf);

		AddrType
		put(char const *data, size_t size);

		AddrType
		put(AddrType addr, char const *data, size_t size);
		
		AddrType
		put(AddrType addr, size_t off, char const* data, size_t size);

		size_t
		get(AddrType addr, char *output, size_t size);
		
		size_t
		get(AddrType addr, size_t off, char *output, size_t size);

		AddrType
		del(AddrType addr);

		AddrType
		del(AddrType addr, size_t off, size_t size);

	private: // disable interfaces
		BDBImpl(BDBImpl const& cp);
		BDBImpl& operator=(BDBImpl const &cp);

	private: // data member
		addr_eval<AddrType> addrEval;

	};

} // end of namespace BDB

#endif // end of header
