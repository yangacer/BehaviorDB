#ifndef _ADDR_EVAL_HPP
#define _ADDR_EVAL_HPP

namespace BDB {

	inline size_t 
	default_chunk_size_est(unsigned char dir, size_t min_size)
	{
		return min_size<<dir;
	}

	inline bool 
	default_capacity_test(size_t chunk_size, size_t data_size)
	{
		return chunk_size>>1 >= data_size;
	}
} // end of namespace BDB

template<typename addr_t>
struct addr_eval
{
	typedef size_t (*Chunk_size_est)(unsigned char dir);

	// Decide how many fragmentation is acceptiable for initial data insertion
	// Or, w.r.t. preallocation, how many fraction of a chunk one want to preserve
	// for future insertion.
	typedef bool (*Capacity_test)(size_t chunk_size, size_t data_size);
	
	unsigned char dir_prefix_len_;
	size_t min_size_;
	
	Chunk_size_est chunk_size_est;
	Capacity_test capacity_test;

	addr_t loc_addr_mask;

	addr_eval(unsigned char dir_prefix_len, size_t min_size, 
		Chunk_size_est cse = &BDB::default_chunk_size_est, 
		Capacity_test ct = &BDB::default_capacity_test )
	: // initialization list
	  dir_bits_len_(dir_prefix_len), min_size_(min_size), 
	  chunk_size_est(cse). capacity_test(ct)
	{
		unsigned char zero_suffix_len = (sizeof(addr_t)-dir_prefix_len);
		loc_addr_mask = ~(((addr_t)(-1))>>zero_suffix_len<<zero_suffix_len);
	}
	
	size_t chunk_size_estimation(unsigned char dir)
	{ return (*chunk_size_est)(dir); }
	

	unsigned char 
	dir_count() const
	{ return 1<<dir_prefix_len_; }

	unsigned char directory(size_t size) const
	{
		for(unsigned char i=0; i < dir_count(); ++i)
			if((*capatict_test)( 
				(*chunk_size_est)(dir, min_size_), 
				size ) ) 
				break;
		return i;
	}
	
	unsigned char addr_to_dir(addr_t addr)
	{
		return addr >> (sizeof(addr_t) - dir_prefix_len_);	
	}

	addr_t global_addr(unsigned char dir, addr_t local_addr) const
	{
		// preservation of failure
		return (local_addr == -1) ? -1 :
			dir<<(sizeof(addr_t)-dir_prefix_len) | (loc_addr_mask & local_addr);
	}

	addr_t local_addr(addr_t global_addr) const
	{ return loc_addr_mask & global_addr; }
};


#endif //end of header 
