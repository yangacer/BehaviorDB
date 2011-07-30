#ifndef _BDB_HPP
#define _BDB_HPP

#include <string>
#include "export.hpp"
#include "common.hpp"

namespace BDB {
	
	struct BDBImpl;
	struct AddrIterator;
	
	struct BDB_EXPORT BehaviorDB
	{
		/** @brief Constructor
		 *  @param conf
		 *  @throw invalid_argument 
		 *  @throw bad_alloc 
		 *  @throw runtime_error
		 *  @throw length_error
		 *  @desc Call conf.validate() internally to verify configuration.
		 */
	        BehaviorDB(Config const & conf);

		~BehaviorDB();
		
		/// TODO deprecate this
		operator void const*() const;

		AddrType
		preserve(size_t preserve_size, char const* data=0, size_t size=0);
		
		/** @brief Put data
		 *  @param data
		 *  @param size
		 *  @return Address of the stored data.
		 */
		AddrType
		put(char const *data, size_t size);
		
		/** @brief Put data to a specific address
		 *  @param data
		 *  @param size
		 *  @param addr
		 *  @param off
		 *  @return Address of the stored data
		 *  @desc If the off parameter is not given, this method acts as an 
		 *  append operation.
		 */
		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=npos);
		
		AddrType
		put(std::string const& data);
		
		AddrType
		put(std::string const& data, AddrType addr, size_t off=npos);
		
		AddrType
		update(char const* data, size_t size, AddrType addr);
		
		AddrType
		update(std::string const& data, AddrType addr);

		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
		size_t
		get(std::string *output, size_t max, AddrType addr, size_t off=0);

		size_t
		del(AddrType addr);

		size_t
		del(AddrType addr, size_t off, size_t size);
	
		stream_state const*
		ostream(size_t stream_size);

		stream_state const*
		ostream(size_t stream_size, AddrType addr, size_t off=npos);

		stream_state const*
		stream_write(stream_state const* state, char const* data, size_t size);
		
		AddrType
		stream_finish(stream_state const* state);

		void
		stream_abort(stream_state const* state);

		AddrIterator
		begin() const;

		AddrIterator
		end() const;
		
		void stat(Stat * ms) const;

	private:
		BehaviorDB(BehaviorDB const& cp);
		BehaviorDB &operator=(BehaviorDB const& cp);

		BDBImpl *impl_;
	};
	
} // end of namespace BDB

#endif
