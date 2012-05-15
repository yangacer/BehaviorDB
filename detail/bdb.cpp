#include "bdb.hpp"
#include "bdbImpl.hpp"
#include "addr_iter.hpp"

namespace BDB {
  
  BehaviorDB::BehaviorDB(Config const &conf)
  : impl_(new BDBImpl(conf))
  {}
  
  BehaviorDB::~BehaviorDB()
  { delete impl_; }

  //BehaviorDB::operator void const*() const
  //{ if(!*impl_) return 0; return this; }
  
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
  
  stream_state const*
  BehaviorDB::ostream(size_t stream_size)
  { return impl_->ostream(stream_size); }

  stream_state const*
  BehaviorDB::ostream(size_t stream_size, AddrType addr, size_t off)
  { return impl_->ostream(stream_size, addr, off); }
  
  stream_state const*
  BehaviorDB::istream(size_t stream_size, AddrType addr, size_t off)
  { return impl_->istream(stream_size, addr, off); }
  
  stream_state const*
  BehaviorDB::stream_write(stream_state const* state, char const* data, size_t size)
  { return impl_->stream_write(state, data, size); }
  
  stream_state const*
  BehaviorDB::stream_read(stream_state const* state, char* output, size_t size)
  { return impl_->stream_read(state, output, size); }

  AddrType
  BehaviorDB::stream_finish(stream_state const* state)
  { return impl_->stream_finish(state); }

  BDBImpl*
  BehaviorDB::impl()
  { return impl_; }

  size_t
  BehaviorDB::stream_pause(stream_state const* state)
  { return impl_->stream_pause(state); }

  stream_state const*
  BehaviorDB::stream_resume(size_t encrypt_handle)
  { return impl_->stream_resume(encrypt_handle); }
  
  void
  BehaviorDB::stream_expire(size_t encrypt_handle)
  { impl_->stream_expire(encrypt_handle); }

  void
  BehaviorDB::stream_abort(stream_state const* state)
  { impl_->stream_abort(state); }

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

