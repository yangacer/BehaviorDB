#ifndef _BDB_HASH_MAP_HPP
#define _BDB_HASH_MAP_HPP

#include <string>

#include "boost/unordered_map.hpp"
#include "boost/noncopyable.hpp"
#include "boost_serialize_unordered_map.hpp"

#include "common.hpp"
#include "typed_array.hpp"

namespace BDB {
namespace Structure {


/*
struct HashFunction
{
  virtual size_t
  operator()(char const* key, size_t len) const =0;
  
  size_t 
  operator()(std::string const& key) const
  { this->operator()(key); }
  
  virtual ~HashFunction();

};
*/

struct Array;

struct HashMap 
: boost::noncopyable
{
private:
  typedef boost::unordered_map<std::string, AddrType> BucketType;
  typedef TypedArray<BucketType> BucketsContType;
  
public:
  
  HashMap(std::string const& name, Array &array, BehaviorDB &bdb);
  HashMap(size_t size, std::string const& name, Array &array, BehaviorDB &bdb);
  ~HashMap();

  bool
  is_in(std::string const& key);

  AddrType
  put(std::string const& key, char const* value, size_t size);
  
  AddrType
  put(std::string const& key, std::string const& value)
  { return put(key, value.data(), value.size()); }

  size_t
  get(std::string const& key, char* buffer, size_t size);

  size_t
  get(std::string const& key, std::string *buffer, size_t max = npos);

  bool
  del(std::string const& key);
  
  // TODO Check why use this return value type
  AddrType
  update(std::string const& key, char const* value, size_t size);

  AddrType
  update(std::string const& key, std::string const& value)
  { return update(key, value.data(), value.size()); }

  bool
  link(std::string const& key, AddrType index);

  bool
  unlink(std::string const& key);

  Array& array()
  { return *arr_; }

protected:

  bool
  get_bucket(std::string const& key, BucketType *bucket);

  AddrType
  put_bucket(std::string const& key, BucketType *bucket);
  /*
  AddrType
  get_index(std::string const& key);
  */

  size_t hash_value(std::string const& key) const
  { return boost::hash<std::string>()(key) % buckets_.size(); }

private:

  Array *arr_;
  BucketsContType buckets_;
  BucketType cvt;
  size_t prev_index_;
};

  
} // namespace Structure
} // namespace BDB 

#endif // header guard
