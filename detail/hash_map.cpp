#include "hash_map.hpp"

namespace BDB {
namespace Structure {
  
  /*
  HashMap::HashMap(std::string const& name, Array &array, BehaviorDB &bdb)
  : arr_(&array) , buckets_(name, bdb), cvt(), prev_index_(-1)
  {}
  */

  HashMap::HashMap(size_t size, std::string const& name, Array &array, BehaviorDB &bdb)
  : arr_(&array), buckets_(size, name, bdb), cvt(), prev_index_(-1)
  {
    assert(size != 0 && "hashmap size can not be zero"); 
  }
  
  HashMap::~HashMap()
  {}

  bool
  HashMap::is_in(std::string const& key)
  {
    if(!get_bucket(key,&cvt))
      return false;
    return cvt.find(key) != cvt.end();
  }
  
  bool
  HashMap::get_bucket(std::string const& key, BucketType *bucket)
  {
    size_t hv = hash_value(key);
    
    if(bucket != &cvt) 
      bucket->clear();
    else if(prev_index_ == hv)
      return true;
    else{
      prev_index_ = hv;
      bucket->clear();
    }
    return buckets_.get(bucket, hv);
  }

  AddrType
  HashMap::put_bucket(std::string const& key, BucketType *bucket)
  {
    size_t hv = hash_value(key);
    return buckets_.update(*bucket, hv);
  }

  AddrType
  HashMap::put(std::string const& key, char const* value, size_t size)
  {
    if(is_in(key)){ // duplicate
      return -1;
    }
    AddrType rt = arr_->put(value, size);
    cvt.insert(std::make_pair(key, rt));
    return buckets_.update(cvt, prev_index_);
  }

  size_t
  HashMap::get(std::string const& key, char* buffer, size_t size)
  {
    if(!is_in(key))
      return -1;
    return arr_->get(buffer, size, (cvt.find(key))->second);
  }

  size_t
  HashMap::get(std::string const& key, std::string *buffer, size_t max)
  {
    if(!is_in(key))
      return -1;
    return arr_->get(buffer, max,( cvt.find(key))->second);
  }
  
  bool
  HashMap::del(std::string const& key)
  {
    if(!is_in(key))
      return false;

    BucketType::iterator i = cvt.find(key);
    if(!arr_->del(i->second))
      return false;
    cvt.erase(i);
    // TODO empty case is not handled
    if(cvt.empty()){
      if(!buckets_.del(prev_index_))
        return false;
    }else  if(-1 == buckets_.update(cvt, prev_index_))
      return false;
    return true;
  }

  AddrType
  HashMap::update(std::string const& key, char const* value, size_t size)
  {
    if(!is_in(key))
      return put(key, value, size);
    BucketType::iterator i = cvt.find(key);
    if(-1 == arr_->update(value, size, i->second))
      return -1;
    return i->second;
  }
  
  bool
  HashMap::link(std::string const& key, AddrType index)
  { 
    if(!is_in(key)){
      cvt.insert(std::make_pair(key, index));
      return -1 != buckets_.update(cvt, prev_index_);
    }
    return false;  
  }
  
  bool
  HashMap::unlink(std::string const& key)
  {
    if(!is_in(key))
      return false; 
    cvt.erase(key);
    return -1 != buckets_.update(cvt, prev_index_);
  }

} // namespace Structure
} // namespace BDB
