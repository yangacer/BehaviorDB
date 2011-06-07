#ifndef _ADDR_EVAL_HPP
#define _ADDR_EVAL_HPP

#include "common.hpp"

namespace BDB {

	
	template<typename addr_t>
	struct addr_eval
	{
		addr_eval(): 
		dir_prefix_len_(0), min_size_(0)
		{}

		addr_eval(unsigned int dir_prefix_len, size_t min_size, 
			Chunk_size_est cse = &BDB::default_chunk_size_est, 
			Capacity_test ct = &BDB::default_capacity_test )
		: // initialization list
		  dir_prefix_len_(dir_prefix_len), min_size_(min_size), 
		  chunk_size_est_(cse), capacity_test_(ct)
		{
			loc_addr_mask = ( (addr_t)(-1) >> local_addr_len()) << local_addr_len();
			loc_addr_mask = ~loc_addr_mask;
		}
		
		operator void const *() const
		{ 
			if((!dir_prefix_len_ && !min_size_) || !chunk_size_est_ || !capacity_test_)
				return 0;
			return this;
		}

		addr_eval&
		set(unsigned char dir_prefix_len)
		{ 
			dir_prefix_len_ = dir_prefix_len;
			loc_addr_mask = ( (addr_t)(-1) >> local_addr_len()) << local_addr_len();
			loc_addr_mask = ~loc_addr_mask;

			return *this;
		}
		
		addr_eval&
		set(size_t min_size)
		{ min_size_ = min_size; return *this; }

		addr_eval&
		set(Chunk_size_est chunk_size_estimation_func)
		{ chunk_size_est_ = chunk_size_estimation_func; return *this; }

		addr_eval&
		set(Capacity_test capacity_test_func)
		{ capacity_test_ = capacity_test_func; return *this; }

		unsigned int
		global_addr_len() const
		{ return dir_prefix_len_; }

		unsigned char
		local_addr_len() const
		{ return (sizeof(addr_t)<<3) - dir_prefix_len_; }

		size_t 
		chunk_size_estimation(unsigned int dir) const
		{ return (*chunk_size_est_)(dir, min_size_); }
		
		bool
		capacity_test(unsigned int dir, size_t size) const
		{ return (*capacity_test_)((*chunk_size_est_)(dir, min_size_), size); }

		unsigned int 
		dir_count() const
		{ return 1<<dir_prefix_len_; }

		unsigned int 
		directory(size_t size) const
		{
			unsigned int i;
			for(i=0; i < dir_count(); ++i)
				if(capacity_test(i, size)) break;

			return i < dir_count() ? i : 
				size <= chunk_size_estimation(i -1) ? i - 1 : -1;
				;
		}
		
		unsigned int 
		addr_to_dir(addr_t addr) const
		{
			return addr >> local_addr_len() ;	
		}

		addr_t 
		global_addr(unsigned int dir, addr_t local_addr) const
		{
			// preservation of failure
			return (local_addr == -1) ? -1 :
				dir << local_addr_len() | (loc_addr_mask & local_addr);
		}

		addr_t 
		local_addr(addr_t global_addr) const
		{ return loc_addr_mask & global_addr; }

	private:
		unsigned char dir_prefix_len_;
		size_t min_size_;
		
		Chunk_size_est chunk_size_est_;
		Capacity_test capacity_test_;

		addr_t loc_addr_mask;

	};

} // end of namespace BDB
#endif //end of header 
