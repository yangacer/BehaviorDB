#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP

#include "config.hpp"
#include "addr_eval.hpp"

namespace BDB {
	
	struct Config
	{
		unsigned char addr_prefix_len;
		
		size_t min_size;
		
		/** Working Directory for BehaviorDB */
		char const * working_dir;

		Chunk_size_est cse_func;

		Capacity_test ct_func;

		/** Setup default configuration  */
		Config()
		: addr_prefix_len(4), min_size(1024), 
		  working_dir(""),
		  cse_func(&default_chunk_size_est), 
		  ct_func(&default_capacity_test)
		{}
		
	};
	
	struct pool;

	struct BDBImpl 
	{

		BDBImpl();
	        BDBImpl(Config const & conf);
		~BDBImpl();
		
		operator void*() const;

		void
		init_(Config const & conf);

		AddrType
		put(char const *data, size_t size);

		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=-1);
		
		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
		AddrType
		del(AddrType addr);

		AddrType
		del(AddrType addr, size_t off, size_t size);

	private: // disable interfaces
		BDBImpl(BDBImpl const& cp);
		BDBImpl& operator=(BDBImpl const &cp);

	public: // data member
		addr_eval<AddrType> addrEval;
	private:
		
		pool* pools_;
	};

} // end of namespace BDB

#endif // end of header
