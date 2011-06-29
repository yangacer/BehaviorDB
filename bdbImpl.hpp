#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP

#include <cstdio>
#include <string>
#include "common.hpp"
#include "addr_eval.hpp"

template<typename B, typename V>
class IDValPool;

namespace BDB {
	
	
	struct pool;
	

	struct BDBImpl 
	{

		BDBImpl();
	        BDBImpl(Config const & conf);
		~BDBImpl();
		
		operator void const*() const;

		void
		init_(Config const & conf);

		AddrType
		put(char const *data, size_t size);

		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=-1);
		
		AddrType
		put(std::string const& data)
		{ return put(data.data(), data.size()); }
		
		AddrType
		put(std::string const& data, AddrType addr, size_t off=-1)
		{ return put(data.data(), data.size(), addr, off); }
		
		AddrType
		update(char const *data, size_t size, AddrType addr);
		
		AddrType
		update(std::string const& data, AddrType addr)
		{ return update(data.data(), data.size(), addr); }

		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
		size_t
		get(std::string *output, size_t max, AddrType addr, size_t off=0);

		size_t
		del(AddrType addr);

		size_t
		del(AddrType addr, size_t off, size_t size);

	protected:

		void
		error(unsigned int dir);
		
		void
		error(int errcode, int line);

	private: // disable interfaces
		BDBImpl(BDBImpl const& cp);
		BDBImpl& operator=(BDBImpl const &cp);

	public: // data member
		addr_eval<AddrType> addrEval;
	private:
		
		pool* pools_;
		FILE* log_;
		char log_buf_[256];

		IDValPool<AddrType, AddrType> *global_id_;

	};

} // end of namespace BDB

#endif // end of header
