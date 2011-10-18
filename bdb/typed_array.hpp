#ifndef TYPED_ARRAY_HPP
#define TYPED_ARRAY_HPP

#include <sstream>
#include "array.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"

namespace BDB {

template<class T, class ArrayImpl=Array>
struct TypedArray 
: private ArrayImpl
{
public:
  using ArrayImpl::resize;
  using ArrayImpl::max_used;
  using ArrayImpl::size;
  using ArrayImpl::del;

  TypedArray(std::string const &name, BehaviorDB &bdb)
  : ArrayImpl(name, bdb)
  {}

  TypedArray(size_t size, std::string const &name, BehaviorDB &bdb)
  : ArrayImpl(size, name, bdb)
  {}
  
  ~TypedArray()
  {}

  AddrType
  put(T const& object)
  {
    cvt.clear();
    cvt.str("");

    boost::archive::binary_oarchive oa(cvt);

    oa << object;
    
    return ArrayImpl::put(cvt.str());
  }
  
  AddrType
  put(T const& object, AddrType index)
  {
    cvt.clear();
    cvt.str("");

    boost::archive::binary_oarchive oa(cvt);
    oa << object;

    return ArrayImpl::put(cvt.str(), index);
  }

  bool
  get(T *object, AddrType index)
  {
    static std::string buf;
    if(0 == ArrayImpl::get(&buf, -1, index))
      return false;
    
    cvt.clear();
    cvt.str(buf);

    boost::archive::binary_iarchive ia(cvt);
    
    ia >> (*object);
    
    return true;
  }

  AddrType
  update(T const& object, AddrType index)
  {
    cvt.clear();
    cvt.str("");

    boost::archive::binary_oarchive oa(cvt);
    oa & object;

    return ArrayImpl::update(cvt.str(), index);
  }

private:
  std::stringstream cvt;

  // TODO cache? tuning via partial reading?
};

} // namespace BDB 

#endif
