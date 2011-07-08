#include "bdb.hpp"
#include "bdbImpl.hpp"
#include "addr_iter.hpp"

namespace BDB {
	
	BehaviorDB::BehaviorDB(Config const &conf)
	: impl_(new BDBImpl(conf))
	{}
	
	BehaviorDB::~BehaviorDB()
	{ delete impl_; }

	BehaviorDB::operator void const*() const
	{ if(!*impl_) return 0; return this; }
	
	AddrType
	BehaviorDB::put(char const *data, size_t size)
	{ return impl_->put(data, size); }

	AddrType
	BehaviorDB::put(char const *data, size_t size, AddrType addr, size_t off)
	{ return impl_->put(data, size, addr, off); }

	AddrType
	BehaviorDB::put(std::string const& data)
	{ return impl_->put(data); }
	
	AddrType
	BehaviorDB::put(std::string const& data, AddrType addr, size_t off)
	{ return impl_->put(data, addr, off); }

	AddrType
	BehaviorDB::update(char const* data, size_t size, AddrType addr)
	{ return impl_->update(data, size, addr); }
	
	AddrType
	BehaviorDB::update(std::string const& data, AddrType addr)
	{ return impl_->update(data, addr); }

	size_t
	BehaviorDB::get(char *output, size_t size, AddrType addr, size_t off)
	{ return impl_->get(output, size, addr, off); }
	
	size_t
	BehaviorDB::get(std::string *output, size_t max, AddrType addr, size_t off)
	{ return impl_->get(output, max, addr, off); }

	size_t
	BehaviorDB::del(AddrType addr)
	{ return impl_->del(addr); }

	size_t
	BehaviorDB::del(AddrType addr, size_t off, size_t size)
	{ return impl_->del(addr, off, size); }

	AddrIterator
	BehaviorDB::begin() const
	{ return impl_->begin(); }

	AddrIterator
	BehaviorDB::end() const
	{ return impl_->end(); }
	
	void
	BehaviorDB::stat(Stat *s) const
	{ impl_->stat(s); }
} // end of namespace BDB

