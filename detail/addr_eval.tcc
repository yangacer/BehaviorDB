#include <cstdio>
#include <limits>

namespace BDB {

	template<typename T>
	unsigned char addr_eval<T>::dir_prefix_len_(0);
	
	template<typename T>
	size_t addr_eval<T>::min_size_(0);
		
	template<typename T>
	Chunk_size_est addr_eval<T>::chunk_size_est_(0);

	template<typename T>
	Capacity_test addr_eval<T>::capacity_test_(0);

	template<typename T>
	T addr_eval<T>::loc_addr_mask(0);

	template<typename T>
	void
	addr_eval<T>::init(unsigned int dir_prefix_len, size_t min_size, 
		Chunk_size_est cse, Capacity_test ct )
	{ 
		dir_prefix_len_ = dir_prefix_len;
		min_size_ = min_size; 
		chunk_size_est_= cse;
		capacity_test_ = ct;

		loc_addr_mask = ( (T)(-1) >> local_addr_len()) << local_addr_len();
		loc_addr_mask = ~loc_addr_mask;
	}

	template<typename T>
	bool
	addr_eval<T>::is_init()
	{ 
		if((!dir_prefix_len_ && !min_size_) || !chunk_size_est_ || !capacity_test_)	
			return false;
		return true;
	}
	
	template<typename T>
	void
	addr_eval<T>::set(unsigned char dir_prefix_len)
	{ 
		dir_prefix_len_ = dir_prefix_len;
		loc_addr_mask = ( (T)(-1) >> local_addr_len()) << local_addr_len();
		loc_addr_mask = ~loc_addr_mask;
	}
	
	template<typename T>
	void
	addr_eval<T>::set(size_t min_size)
	{ min_size_ = min_size;}

	template<typename T>
	void
	addr_eval<T>::set(Chunk_size_est chunk_size_estimation_func)
	{ chunk_size_est_ = chunk_size_estimation_func;}

	template<typename T>
	void
	addr_eval<T>::set(Capacity_test capacity_test_func)
	{ capacity_test_ = capacity_test_func; }


	template<typename T>
	unsigned int
	addr_eval<T>::global_addr_len()
	{ return dir_prefix_len_; }

	template<typename T>
	unsigned char
	addr_eval<T>::local_addr_len()
	{ return (sizeof(T)<<3) - dir_prefix_len_; }

	template<typename T>
	size_t 
	addr_eval<T>::chunk_size_estimation(unsigned int dir)
	{ return (*chunk_size_est_)(dir, min_size_); }
	
	template<typename T>
	bool
	addr_eval<T>::capacity_test(unsigned int dir, size_t size)
	{ return (*capacity_test_)((*chunk_size_est_)(dir, min_size_), size); }

	template<typename T>
	unsigned int 
	addr_eval<T>::dir_count()
	{ return 1<<dir_prefix_len_; }

	template<typename T>
	unsigned int 
	addr_eval<T>::directory(size_t size)
	{
		unsigned int i;
		for(i=0; i < dir_count(); ++i)
			if(capacity_test(i, size)) break;

		return i < dir_count() ? i : 
			size <= chunk_size_estimation(i -1) ? i - 1 : -1;
			;
	}
	
	template<typename T>
	unsigned int 
	addr_eval<T>::addr_to_dir(T addr)
	{
		return addr >> local_addr_len() ;	
	}

	template<typename T>
	T 
	addr_eval<T>::global_addr(unsigned int dir, T local_addr)
	{
		// preservation of failure
		return (local_addr == -1) ? -1 :
			dir << local_addr_len() | (loc_addr_mask & local_addr);
	}

	template<typename T>
	T
	addr_eval<T>::local_addr(T global_addr)
	{ return loc_addr_mask & global_addr; }
	

} // end of namespace BDB
