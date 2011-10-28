#ifndef _BDB_ARRAY_HPP
#define _BDB_ARRAY_HPP

#include <string>
#include <vector>
#include <iosfwd>
#include "boost/dynamic_bitset.hpp"
#include "boost/noncopyable.hpp"
#include "bdb.hpp"

namespace BDB {
namespace Structure {

  struct Array;
  
  // TODO: Shared Array issue
  // Replace all return type with AddrHandle?
  /*
  struct AddrHandle
  {
    friend struct Array;
  private:  
    AddrHandle(AddrType index, Array const* array);
    AddrType idx_;
    Array const* arr_;
  };
  */

	/// @todo TODO: Transaction file compression (snapshot).
  class Array : boost::noncopyable
	{

	protected:
		typedef boost::dynamic_bitset<AddrType> Bitmap;
	  typedef std::vector<AddrType> AddrContainer;
  public:
    
    Array(std::string const& name, BehaviorDB &bdb);
    Array(size_t size, std::string const& name, BehaviorDB &bdb);
    virtual ~Array();
    
    //bool
    //is_in(AddrHandle const& handle) const;
  
    AddrType
    put(char const* data, size_t size);

		AddrType 
		put(char const *data, size_t size, AddrType index, AddrType offset=npos);
    
    AddrType
    put(std::string const& data)
    { return put(data.data(), data.size()); }
  
    AddrType
    put(std::string const& data, AddrType index, AddrType offset=npos)
    { return put(data.data(), data.size(), index, offset); }
    
    size_t
    get(char *buffer, size_t size, AddrType index, size_t offset=0);
    
    size_t
    get(std::string *buffer, size_t max, AddrType index, size_t offset=0);

	  bool 
		del(AddrType index);
		
    AddrType
    update(char const* data, size_t size, AddrType index);
    
    AddrType
    update(std::string const& data, AddrType index)
    { return update(data.data(), data.size(), index); }

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

    // --------- Oberservers ----------
		bool 
		is_acquired(AddrType index) const
    { return bm_[index] == false; }
    
		bool
    is_locked(AddrType index) const
    { return lock_[index]; }

#ifndef _BDB_TESTING_
  protected:
#endif   

    void 
		replay_transaction(std::string const& file);
		
		void 
		init_transaction(std::string const& file);

#ifndef _BDB_TESTING_
  private:
#endif

    AddrType
    acquire();

    bool
    acquire(AddrType index);

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

    
	  std::ofstream *ofs_;
    Bitmap bm_;
		Bitmap lock_;
		AddrType max_used_;
		char filebuf_[128];
    
    std::vector<AddrType> arr_;
    BehaviorDB &bdb_;

	};

} // namespace Structure
} // end of namespace BDB

#endif // header guard


