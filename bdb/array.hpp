#ifndef _IDPOOL_HPP
#define _IDPOOL_HPP

#include <string>
#include <vector>
#include <iosfwd>
#include "boost/dynamic_bitset.hpp"
#include "boost/noncopyable.hpp"
#include "common.hpp"
#include "bdb.hpp"

namespace BDB {

	/// @todo TODO: Transaction file compression (snapshot).
  class Array : boost::noncopyable
	{

	protected:
		typedef boost::dynamic_bitset<AddrType> Bitmap;
	
  public:
    
    // @todo TODO: anonymous Array?
    
    Array(std::string const& name, BehaviorDB &bdb);
    Array(size_t size, std::string const& name, BehaviorDB &bdb);
    ~Array();
  
  /*
    AddrType
    put(char const* data, size_t size);

		AddrType 
		put(char const *data, size_t size, AddrType index);
    
    size_t
    get(char *buffer, size_t size, AddrType index, size_t offset);
    
	  bool 
		del(AddrType index);
		
    AddrType
    update(char const* data, size_t size, AddrType index);
*/
    void
    resize(size_t size);

		/** Find the first acquired ID from cur which is included
		 * @param cur Current index
		 * @remark The cur will be tested also.
		 */
		//AddrType 
		//next_used(AddrType cur) const;
		
		/** The maximum count of used IDs */
		AddrType
		max_used() const
    { return max_used_; }

		size_t 
		size() const;
	
  //protected:
    
    void 
		replay_transaction(std::string const& file);
		
		void 
		init_transaction(std::string const& file);

		//int 
		//write_transaction(char const* data, size_t size);
    
  //private:
    
    AddrType
    acquire();

    bool
    acquire(AddrType index, AddrType addr);

    void
    release(AddrType index);
  
    bool
		commit(AddrType index);

		void
		lock(AddrType index)
    { lock_[index] = true; }

    void
    unlock(AddrType index)
    { lock_[index] = false; }

    void
    remap_address(AddrType index, AddrType internal);

    // --------- Oberservers ----------
		bool 
		is_acquired(AddrType index) const
    { return bm_[index] == false; }
    
		bool
    is_locked(AddrType index) const
    { return lock_[index]; }

	  std::ofstream *ofs_;
    Bitmap bm_;
		Bitmap lock_;
		AddrType max_used_;
		char filebuf_[128];
    
    std::vector<AddrType> arr_;
    BehaviorDB &bdb_;

	};

} // end of namespace BDB

#endif


