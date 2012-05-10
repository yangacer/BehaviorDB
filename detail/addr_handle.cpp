#include "addr_handle.hpp"
#include "idPool.hpp"
#include <stdexcept>
#include "error.hpp"

namespace BDB {

  addr_handle::addr_handle(IDPool &idp)
  : idp_(idp), addr_(-1), commited_(false)
  {
    if(-1 == (addr_ = idp_.Acquire()))
      throw addr_overflow();
  }
  
  /* XXX not sure to have this
  addr_handle::addr_handle(IDPool &idp, AddrType addr)
  : idp_(idp), addr_(addr), commitd_(false)
  {
    
  }
  */

  addr_handle::~addr_handle()
  {
    if(!commited_)
      idp_.Release(addr_);
  }

  AddrType addr_handle::addr() const
  {
    return addr_;
  }
  
  void addr_handle::commit()
  {
    if(false == (commited_ = idp_.Commit(addr_)))
      throw std::runtime_error(SRC_POS);
  }
}
