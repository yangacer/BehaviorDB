#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP

#include "config.hpp"
#include "addr_eval.hpp"

namespace BDB {
	
	struct Config
	{
		unsigned int addr_prefix_len;
		
		size_t min_size;
		
		char const *pool_dir;
		char const *trans_dir;
		char const *header_dir;
		char const *log_dir;

		Chunk_size_est cse_func;

		Capacity_test ct_func;

		/** Setup default configuration  */
		Config()
		: addr_prefix_len(4), min_size(1024), 
		  pool_dir(""), trans_dir(""), header_dir(""), log_dir(""),
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
		
		operator void const*() const;

		void
		init_(Config const & conf);

		AddrType
		put(char const *data, size_t size);

		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=-1);
		
		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
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
	};

} // end of namespace BDB

#endif // end of header
