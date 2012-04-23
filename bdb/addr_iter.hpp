#ifndef _ADDR_ITER_HPP
#define _ADDR_ITER_HPP

#include "export.hpp"
#include "common.hpp"

namespace BDB {

  struct BDBImpl;

  /** @brief Iterating addresses
  */
  struct BDB_EXPORT AddrIterator 
  {
    friend struct BDBImpl;

    /// ctor
    AddrIterator();

    /// copy ctor
    AddrIterator(AddrIterator const &cp);

    /// Assignment operator
    AddrIterator& 
    operator=(AddrIterator const& cp);

    /// prefix increment operator
    AddrIterator &
    operator++();

    /** Get the address on iterating
    */
    AddrType
    operator*() const;

    /// equality binary operator
    bool 
    operator==(AddrIterator const& rhs) const;

    /// in-equality binary operator
    bool 
    operator!=(AddrIterator const& rhs) const
      { return !(*this == rhs); }

    protected:

    /// internal ctor
    AddrIterator(BDBImpl const &bdb, AddrType cur);

  private:
    AddrType cur_;
    BDBImpl const *bdb_;

  };
} // end of namespace BDB

#endif // end of header
