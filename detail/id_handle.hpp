#ifndef BDB_ID_HANDLE_HPP_
#define BDB_ID_HANDLE_HPP_

#include "common.hpp"
#include <boost/noncopyable.hpp>

namespace BDB {

  namespace detail{
    enum IDOperation{
      ACQUIRE_AUTO = 0,
      ACQUIRE_SPEC = 1,
      RELEASE = 2,
      MODIFY = 4,
      READONLY = 8
    };
  }

  template<class IDPool_>
  struct id_handle
  {
    typedef typename IDPool_::value_type value_type;

    id_handle(
      detail::IDOperation op,
      IDPool_ &idp);
    
    id_handle(
      detail::IDOperation op,
      IDPool_ &idp, AddrType addr);

    ~id_handle();
    
    AddrType addr() const;
    value_type const& const_value() const;
    value_type &value();
    void commit();

  private:
    detail::IDOperation op_;
    IDPool_ &idp_;
    AddrType addr_;
    value_type val_;
    bool commited_;
  };

} // namespace BDB

#endif // header guard
