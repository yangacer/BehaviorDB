#include "id_handle.hpp"
#include "exception.hpp"
#include "error.hpp"
#include <stdexcept>

namespace BDB {
  
template<class IDP>
id_handle<IDP>::id_handle(
  detail::IDOperation op,
  IDP &idp)
: op_(op), idp_(idp), addr_(), val_(), 
  commited_(false)
{
  addr_ = idp_.template Acquire();
}

template<class IDP>
id_handle<IDP>::id_handle(
  detail::IDOperation op,
  IDP &idp,
  AddrType addr)
: op_(op), idp_(idp), addr_(), val_(), 
  commited_(false)
{
  using namespace detail;

  switch(op){
  case ACQUIRE_SPEC:
    if(idp_.template isAcquired(addr))
      throw invalid_addr();
    addr_ = idp_.template Acquire(addr);
    break;
  case RELEASE:
    if(false == idp_.template isAcquired(addr))
      throw invalid_addr();
    idp_.template Release(addr);
    addr_ = addr;
    break;
  case MODIFY:
  case READONLY:
    if(false == idp_.template isAcquired(addr))
      throw invalid_addr();
    addr_ = addr;
    val_ = idp_.template Find(addr);
    break;
  default:
    throw std::invalid_argument(SRC_POS);
  }
}

template<class IDP>
id_handle<IDP>::~id_handle()
{
  using namespace detail;
  if(commited_) return;

  unsigned int opcode = (unsigned int)op_;
  switch(op_){
  case ACQUIRE_AUTO:
  case ACQUIRE_SPEC:
    idp_.template Release(addr_);
    break;
  case RELEASE:
    idp_.template Acquire(addr_);
    break;
  default: // MODIFY and READONLY are OK
    break;
  }
}

template<class IDP>
AddrType id_handle<IDP>::addr() const
{ return addr_; }

template<class IDP>
typename id_handle<IDP>::value_type const &
id_handle<IDP>::const_value() const
{  return val_; }

template<class IDP>
typename id_handle<IDP>::value_type &
id_handle<IDP>::value() 
{ 
  if(op_ == detail::READONLY) 
    throw std::runtime_error("Access value of READONLY ID handle");
  return val_;
}

template<class IDP>
void id_handle<IDP>::commit()
{
  commited_ = 
    detail::READONLY == op_ ?
    true 
    : idp_.template Commit(addr_, val_);
}

} // namespace BDB
