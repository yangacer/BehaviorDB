#ifndef _COMMON_HPP
#define _COMMON_HPP

#ifdef __GNUC__ // GNU

#define PATH_DELIM '/'

#elif defined(_WIN32) // MSVC

#define PATH_DELIM '\\'

// ftello/fseeko
#define ftello(X) _ftelli64(X)
#define fseeko(X,Y,Z) _fseeki64(X,Y,Z)

#pragma warning( disable: 4290 ) // exception specification non-implmented
#pragma warning( disable: 4251 ) // template export warning
#pragma warning( disable: 4996 ) // allow POSIX 

#endif

/// TODO make sure this is OK in win
#include <stddef.h>

namespace BDB {
	
	typedef unsigned int AddrType;
	struct stream_state;
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
	{	return chunk_size >= data_size;	}
	
	/** @brief Configuration of BehaviorDB */
	struct Config
	{
		/// Begin number of global IDs of BehaviorDB
		AddrType beg;
		
		/// End number of global IDs of BehaviorDB (the one after the last ID)
		AddrType end;

		/// Bit length of address prefixes. 
		/** This parameter limits how many pools a BehaviorDB can have.
		 */
		unsigned int addr_prefix_len;

		/// Minimum chunk size of BehaviorDB 
		size_t min_size;
		
		//! Directory for placing files that are used by BehaviorDB. 
		/** Default is an empty string so that a BehaviorDB uses current
		 *  directory as a root dir.
		 */
		char const *root_dir;

		/// Directory for placing pool files. Default is the root_dir
		char const *pool_dir;

		/// Directory for placing transaction files. Default is the root_dir.
		char const *trans_dir;

		/// Directory for placing header files. Default is the root_dir.
		char const *header_dir;

		/// Directory for placing log file. Default is the root_dir.
		char const *log_dir;

		Chunk_size_est cse_func;

		Capacity_test ct_func;

		/** @brief Config default constructor 
		 * @desc Construct BDB::Config with default configurations  
		 */
		Config()
		: beg(1), end(100000001),
		  addr_prefix_len(4), min_size(32), 
		  root_dir(""), pool_dir(""), trans_dir(""), header_dir(""), log_dir(""),
		  cse_func(&default_chunk_size_est), 
		  ct_func(&default_capacity_test)
		{ validate(); }
		
		/** @brief Validate configuration
		 *  @throw invalid_argument
		 */
		void
		validate() const;
	};
	
	/// Memory/Disk Statistic
	struct Stat
	{
		/// global ID table byte size
		unsigned long long gid_mem_size;

		/// pool byte size
		unsigned long long pool_mem_size;
			
		unsigned long long disk_size;

		Stat():gid_mem_size(0), pool_mem_size(0), disk_size(0)
		{}
	};
	
	/// Not a Position
	extern const size_t npos;

	/// Version information
	extern char const* VERSION;

} // end of nemaespace BDB

#endif // end of header
