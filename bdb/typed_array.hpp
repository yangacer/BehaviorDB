#ifndef TYPED_ARRAY_HPP
#define TYPED_ARRAY_HPP

#include <sstream>
#include "array.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"

namespace BDB {
namespace Structure {

template<class T, class ArrayImpl=Array>
struct TypedArray 
: private ArrayImpl
{
  using ArrayImpl::resize;
  using ArrayImpl::max_used;
  using ArrayImpl::size;
  using ArrayImpl::del;
  using ArrayImpl::is_acquired;

  typedef boost::archive::text_oarchive oarchiver;
  typedef boost::archive::text_iarchive iarchiver;

  
  TypedArray(std::string const &name, BehaviorDB &bdb)
  : ArrayImpl(name, bdb), cvt()
  {}

  TypedArray(size_t size, std::string const &name, BehaviorDB &bdb)
  : ArrayImpl(size, name, bdb), cvt()
  {}
  
  ~TypedArray()
  {}

  AddrType
  put(T const& object)
  {
    cvt.clear();
    cvt.str("");

    oarchiver oa(cvt);

    oa << object;
    
    return ArrayImpl::put(cvt.str());
  }
  
  AddrType
  put(T const& object, AddrType index)
  {
    cvt.clear();
    cvt.str("");

    oarchiver oa(cvt);
    oa << object;

    return ArrayImpl::put(cvt.str(), index);
  }

  bool
  get(T *object, AddrType index)
  {
    std::string buf;
    if(0 == ArrayImpl::get(&buf, -1, index))
      return false;
    
    cvt.clear();
    cvt.str(buf);

    iarchiver ia(cvt);
    
    ia >> (*object);
    
    return true;
  }

  AddrType
  update(T const& object, AddrType index)
  {
    cvt.clear();
    cvt.str("");

    oarchiver oa(cvt);
    oa << object;

    return ArrayImpl::update(cvt.str(), index);
  }

private:
  std::stringstream cvt;

  // TODO cache? tuning via partial reading?
};
} // namespace Structure
} // namespace BDB 

#endif
