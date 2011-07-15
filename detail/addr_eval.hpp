#ifndef _ADDR_EVAL_HPP
#define _ADDR_EVAL_HPP

#include "common.hpp"

namespace BDB {

	
	template<typename addr_t = AddrType>
	struct addr_eval
	{
		static void
		init( 	unsigned int dir_prefix_len, size_t min_size, 
			Chunk_size_est cse = &BDB::default_chunk_size_est, 
			Capacity_test ct = &BDB::default_capacity_test );

		static bool
		is_init();
			
		static void
		set(unsigned char dir_prefix_len);
		
		static void
		set(size_t min_size);

		static void
		set(Chunk_size_est chunk_size_estimation_func);

		static void
		set(Capacity_test capacity_test_func);

		static unsigned int
		global_addr_len();

		static unsigned char
		local_addr_len();

		static size_t 
		chunk_size_estimation(unsigned int dir);
		
		static bool
		capacity_test(unsigned int dir, size_t size);

		static unsigned int 
		dir_count();
		
		// estimate directory ID according to chunk size
		static unsigned int 
		directory(size_t size);
		
		static unsigned int 
		addr_to_dir(addr_t addr);

		static addr_t 
		global_addr(unsigned int dir, addr_t local_addr);

		static addr_t 
		local_addr(addr_t global_addr);
		
	private:
		static unsigned char dir_prefix_len_;
		static size_t min_size_;
		
		static Chunk_size_est chunk_size_est_;
		static Capacity_test capacity_test_;

		static addr_t loc_addr_mask;

	};

} // end of namespace BDB

#include "addr_eval.tcc"

#endif //end of header 
