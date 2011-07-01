#ifndef _BDB_HPP
#define _BDB_HPP

#include <string>
#include "common.hpp"

namespace BDB {
	
	struct BDBImpl;
	struct AddrIterator;

	struct BehaviorDB
	{
	        BehaviorDB(Config const & conf);
		~BehaviorDB();
		
		operator void const*() const;

		AddrType
		put(char const *data, size_t size);

		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=-1);
		
		AddrType
		put(std::string const& data);
		
		AddrType
		put(std::string const& data, AddrType addr, size_t off=-1);
		
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

		AddrIterator
		begin() const;

		AddrIterator
		end() const;

	private:
		BehaviorDB(BehaviorDB const& cp);
		BehaviorDB &operator=(BehaviorDB const& cp);

		BDBImpl *impl_;
	};
	
} // end of namespace BDB

#endif
