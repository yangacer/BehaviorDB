#ifndef BDB_ADDR_HANDLE_HPP_
#define BDB_ADDR_HANDLE_HPP_

#include "common.hpp"
#include <boost/noncopyable.hpp>

namespace BDB{

  class IDPool;

  struct addr_handle
  : boost::noncopyable
  {
    addr_handle(IDPool &idp);
    //addr_handle(IDPool &idp, AddrType addr);
    ~addr_handle();

    AddrType addr() const;
    void commit();

  private:
    IDPool &idp_;
    AddrType addr_;
    bool commited_;
    
  };

} // namespace BehaviorDB
#endif // header guard
