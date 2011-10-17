#ifndef _IDPOOL_HPP
#define _IDPOOL_HPP

#include <cstdio>
#include <limits>
#include "boost/dynamic_bitset.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "common.hpp"
#include "bdb.hpp"

namespace BDB {

	/// @todo TODO: Transaction file compression (snapshot).

	/** @brief Integer ID manager within bitmap storage.
	 */

    
  class Array : boost::noncopyable
	{

	protected:
		typedef AddrType BlockType;
		typedef boost::dynamic_bitset<BlockType> Bitmap;
	
  public:
        
		Array();
    Array(size_t size, std::string const& name);
    ~Array();

		bool 
		is_assigned(AddrType index) const;
    
    AddrType
    put(char const* data, size_t size);

		AddrType 
		put(char const *data, size_t size, AddrType index);
    
    size_t
    get(char *buffer, size_t size, AddrType index, size_t offset);
    
		bool 
		avail() const;

	  bool 
		del(AddrType index);
		
    AddrType
    update(char const* data, size_t size, AddrType index);

		/** Find the first acquired ID from cur which is included
		 * @param cur Current index
		 * @remark The cur will be tested also.
		 */
		AddrType 
		next_used(AddrType cur) const;
		
		/** The maximum count of used IDs */
		AddrType
		max_used() const;

		size_t 
		size() const;
		
    /*
		AddrType 
    begin() const;

		AddrType 
    end() const;
		*/
	protected:
		void 
		replay_transaction(char const* transaction_file);
		
		void 
		init_transaction(char const* transaction_file);

		int 
		write_transaction(char const* data, size_t size);
		
		/** Extend bitmap size to 1.5 times large
		 *  @throw std::bad_alloc
		 */
		void extend(Bitmap::size_type new_size=0);
    
  private:
    
    bool
		commit(AddrType index);

		void
		lock(AddrType index);

		void
		unlock(AddrType index);
		
		bool
		is_locked(AddrType index) const;

    void
    remap_address(AddrType index, AddrType internal);

    size_t const size_;
		FILE*  file_;
		Bitmap bm_;
		Bitmap lock_;
		AddrType max_used_;
		char filebuf_[128];
    
    AddrType *arr_;
    BehaviorDB &bdb_;

	};

} // end of namespace BDB

#endif


