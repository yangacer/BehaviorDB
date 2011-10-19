#include "array.hpp"
#include <stdexcept>
#include <fstream>
#include "boost/lexical_cast.hpp"
#include "bdbImpl.hpp"

namespace BDB {

  Array::Array(std::string const& name, BehaviorDB &bdb)
  : ofs_(0), bm_(), lock_(), max_used_(0), filebuf_(),
    arr_(), bdb_(bdb)
  {
    if(!name.size())
      throw std::invalid_argument("name is not given");

    ofs_ = new std::ofstream();

    replay_transaction(name);
    init_transaction(name);
  }

  Array::Array(size_t size, std::string const& name, BehaviorDB &bdb)
  : ofs_(0), bm_(), lock_(), max_used_(0), filebuf_(),
    arr_(), bdb_(bdb)
  {
    if(!name.size())
      throw std::invalid_argument("name is not given");
    
    resize(size);

    ofs_ = new std::ofstream();

    replay_transaction(name);
    init_transaction(name);
  }
  
  Array::~Array()
  {
    ofs_->close();
    delete ofs_;
  }
  
  
  AddrType
  Array::put(char const* data, size_t size)
  {
    AddrType index = acquire(), addr(-1);
    if(-1 == index)
      return -1;
    addr = bdb_.impl()->nt_put(data, size);
    if(-1 == addr){
      release(index);
      return -1;
    }
    arr_[index] = addr;
    if(!commit(index)){
      if(-1 == bdb_.impl()->nt_del(addr))
        throw std::runtime_error("Rollback failure");
      release(index);
      return -1;
    }
    return index;
  }
  
  
  AddrType 
  Array::put(char const *data, size_t size, AddrType index, AddrType offset)
  {
    AddrType addr;
    if(!is_acquired(index)){
      if(!acquire(index))
        return -1;
      addr = bdb_.impl()->nt_put(data, size);
      if(-1 == addr){
        release(index);
        return -1;
      } 
    }else{
      addr = bdb_.impl()->nt_put(data, size, arr_[index], offset);
      if(-1 == addr)
        return -1;
    }
    
    AddrType p_addr(arr_[index]);
    if(addr != p_addr){
      arr_[index] = addr;
      if(!commit(index)){
          arr_[index] = p_addr;
          return -1;
      }
    }
    return index;
  }

  size_t
  Array::get(char *buffer, size_t size, AddrType index, size_t offset)
  {
    if(!is_acquired(index)) return 0;
    return bdb_.impl()->nt_get(buffer, size, arr_[index], offset);
  }

  size_t
  Array::get(std::string *buffer, size_t max, AddrType index, size_t offset)
  {
    if(!is_acquired(index))
      return 0;
    return bdb_.impl()->nt_get(buffer, max, arr_[index], offset);
  }

  bool
  Array::del(AddrType index)
  {
    if(!is_acquired(index)) return false;
    
    if(-1 == bdb_.impl()->nt_del(arr_[index]))
      return false;
    
    release(index);

    if(!commit(index))
      throw std::runtime_error("Commit error");
    return true;  
  }

  AddrType
  Array::update(char const* data, size_t size, AddrType index)
  {
    if(!is_acquired(index))
      return put(data, size, index);
    
    AddrType addr, p_addr = arr_[index];
    addr = bdb_.impl()->nt_update(data, size, arr_[index]);
    
    if(-1 == addr)
      return -1;
    
    if(p_addr != addr){
      arr_[index] = addr;
      if(!commit(index)){
        arr_[index] = p_addr;
        return -1;
      }
    }
    return index;
  }

  void
  Array::resize(size_t size)
  {
    if(size == Bitmap::npos)
      throw std::bad_alloc();
    bm_.resize(size, true);
    lock_.resize(size, false);
    arr_.resize(size);
  }
  
  AddrType
  Array::acquire()
  {
    AddrType index = 
      (max_used_) ?
      bm_.find_next(max_used_ - 1) :
      bm_.find_first();

    if((AddrType)Bitmap::npos == index){
      try{
        resize((unsigned int)((size() + 1)<<1)); 
        index = max_used_;
        max_used_++;
      }catch(std::bad_alloc const& e){
        index = bm_.find_first();
        if((AddrType)Bitmap::npos == index)
          return -1;
      } 
    }
    bm_[index] = false;
    return index;
  }
  
  bool
  Array::acquire(AddrType index)
  {
    if(index < size()){
      if(!bm_[index]) return false;
    }else{
      try{
        resize((unsigned int)((index + 1)*1.5)); 
        max_used_++;
      }catch(std::bad_alloc const& e){
        return false;
      }
    }
    bm_[index] = false;
    return true;
  }

  void
  Array::release(AddrType index)
  {
    assert(index < size());
    bm_[index] = true;
    return;
  }

  bool
  Array::commit(AddrType index)
  {
    using boost::lexical_cast;
    using std::string;

    string buf;
    
    if(bm_[index])
      buf = "- " + lexical_cast<string>(index);
    else{
      buf += "+ ";
      buf += lexical_cast<string>(index);
      buf += "\t";
      buf += lexical_cast<string>(arr_[index]);
    }
    
    buf += "\n";
    return (void*)ofs_->write(buf.c_str(), buf.size());    

  }
  
  void
  Array::replay_transaction(std::string const& file)
  {
    using std::string;
    using std::ios;
    using boost::lexical_cast;
    using std::ifstream;

    ifstream ifs(file.c_str(), ios::binary | ios::in);
    
    if(!ifs.is_open())
      return;

    string tok;
    AddrType index, addr;
    while(ifs>>tok){
      if("+" == tok){
        ifs>>tok;
        assert(true != ifs.fail());
        index = lexical_cast<AddrType>(tok);
        ifs>>tok;
        assert(true != ifs.fail());
        addr = lexical_cast<AddrType>(tok);
        if(index >= max_used_)
          max_used_ = index + 1;
        if(index >= size())
          resize(index + 1);
        bm_[index] = false;
        arr_[index] = addr;
      }else if("-" == tok){
        ifs>>tok;
        assert(true != ifs.fail());
        index = lexical_cast<AddrType>(tok);
        bm_[index] = true;
      }else{
        assert("replay failure");  
      }
    }
    ifs.close();
  }
  
  void
  Array::init_transaction(std::string const& file)
  {
    using std::ios;

    assert(0 != ofs_ && 
      "ofstream is not proper allocated");
    
    ofs_->rdbuf()->pubsetbuf(filebuf_, 128);
    ofs_->open(file.c_str(), ios::out | ios::app);
    if(!ofs_->is_open())
      throw std::runtime_error(
        "faile to create array file");

  }

  size_t
  Array::size() const
  {
    return bm_.size();  
  }

} // namespace BDB
