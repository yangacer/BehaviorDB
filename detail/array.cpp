#include "array.hpp"
#include <stdexcept>
#include <fstream>
#include "boost/lexical_cast.hpp"

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
        resize((unsigned int)((size() + 1)*1.5)); 
        index = bm_.find_next(max_used_ -1);
        max_used_ ++;
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
  Array::acquire(AddrType index, AddrType addr)
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
    arr_[index] = addr;
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
    ofs_->write(buf.c_str(), buf.size());    

    return true;
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
