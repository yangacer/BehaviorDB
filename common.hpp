#ifndef _COMMON_HPP
#define _COMMON_HPP

#ifdef __GNUC__ // GNU

#elif defined(_WIN32) // MSVC

// ftello/fseeko
#define ftello(X) _ftelli64(X)
#define fseeko(X,Y,Z) _fseeki64(X,Y,Z)

#pragma warning( disable: 4290 ) // exception specification non-implmented
#pragma warning( disable: 4251 ) // template export warning
#pragma warning( disable: 4996 ) // allow POSIX 

#endif

// TODO make sure this is OK in win
#include <stddef.h>

namespace BDB {
	
	typedef unsigned int AddrType;
	typedef size_t (*Chunk_size_est)(unsigned int dir, size_t min_size);

	// Decide how many fragmentation is acceptiable for initial data insertion
	// Or, w.r.t. preallocation, how many fraction of a chunk one want to preserve
	// for future insertion.
	// *****
	// It should return true if data/chunk_size is acceptiable, otherwise it return false
	typedef bool (*Capacity_test)(size_t chunk_size, size_t data_size);

	inline size_t 
	default_chunk_size_est(unsigned int dir, size_t min_size)
	{
		return min_size<<dir;
	}

	inline bool 
	default_capacity_test(size_t chunk_size, size_t data_size)
	{
		if(chunk_size >= 10000000) return true;
		// reserve 1/4 chunk size
		return (chunk_size - (chunk_size>>2)) >= data_size;
	}

	struct Config
	{
		AddrType beg, end;
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
		: beg(1), end(100000001),
		  addr_prefix_len(4), min_size(32), 
		  pool_dir(""), trans_dir(""), header_dir(""), log_dir(""),
		  cse_func(&default_chunk_size_est), 
		  ct_func(&default_capacity_test)
		{}
		
	};

} // end of nemaespace BDB

#endif // end of header
