#ifndef _BDB_STAT_HPP
#define _BDB_STAT_HPP

#include "common.hpp"


namespace BDB {

  template<class T>
  class IDPool;
  struct BDBImpl;
  struct pool;

  struct bdbStater 
  {
    bdbStater(Stat *s);

    void
    operator()(BDBImpl const* bdb) const;
    
    void
    operator()(pool const *pool) const;
    
    template<typename T>
    void
    operator()( IDPool<T> const *idp) const;

    Stat *s;
  };

} // end of namespace BDB

#endif // end of header
