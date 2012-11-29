#include "fixedPool.hpp"
#include "chunk.h"
#include "addr_wrapper.hpp"
#include "file_utils.hpp"
#include <stdexcept>
#include <cassert>
#include <cstring>
#include "error.hpp"

namespace BDB {

template<typename T, uint32_t TextSize>
fixed_pool<T,TextSize>::fixed_pool(uint32_t) 
: id_(0), work_dir_(""), file_(0), fbuf_(0)
{}

template<typename T, uint32_t TextSize>
fixed_pool<T,TextSize>::fixed_pool(uint32_t id, char const* work_dir)
  : id_(0), work_dir_(""), file_(0), fbuf_(0)
{
  open(id, work_dir);
}

template<typename T, uint32_t TextSize>
fixed_pool<T,TextSize>::~fixed_pool()
{
  if(file_) fclose(file_);
  delete []fbuf_;
}

template<typename T, uint32_t TextSize>
fixed_pool<T,TextSize>::operator void const *() const
{ 
  if(!file_) return 0;
  return this;
}

template<typename T, uint32_t TextSize>
void
fixed_pool<T,TextSize>::open(uint32_t id, char const* work_dir)
{
  using namespace std;

  id_ = id;
  work_dir_ = work_dir;

  char fname[256];
  if(work_dir_.size() > 256) 
    throw length_error("fixed_pool: length of pool_dir string is too long");

  sprintf(fname, "%s%04x.fpo", work_dir_.c_str(), id_);

  fbuf_ = new char[4096];

  if(0 == (file_ = fopen(fname, "r+b"))){
    if(0 == (file_ = fopen(fname, "w+b"))){
      string msg("fixed_pool: Unable to create fix pool ");
      msg += fname;
      throw invalid_argument(msg.c_str());
    }
  }
  setvbuf(file_, fbuf_, _IOFBF, 4096);

}

template<typename T, uint32_t TextSize>
int fixed_pool<T,TextSize>::read(T* val, AddrType addr) const
{
  if(!*this) return -1;

  off_t loc_addr = addr;
  loc_addr *= TextSize;
  fseeko(file_, loc_addr, SEEK_SET);
  if( 0 == file_>>*val || ferror(file_))
    throw std::runtime_error(SRC_POS);
  return 0;
}

template<typename T, uint32_t TextSize>
int fixed_pool<T,TextSize>::write(T const & val, AddrType addr)
{
  if(!*this) return -1;

  off_t loc_addr = addr;
  loc_addr *= TextSize;
  fseeko(file_, loc_addr, SEEK_SET);
  if( 0 == file_<<val || ferror(file_) || fflush(file_) ) 
    throw std::runtime_error(SRC_POS);
  return 0;
}

template<typename T, uint32_t TextSize>
std::string
fixed_pool<T,TextSize>::dir() const 
{
  return work_dir_;
}


template<typename T, uint32_t TextSize>
T fixed_pool<T,TextSize>::operator[](AddrType addr) const
{
  T rt;
  read(&rt, addr);
  return rt;
}

template<typename T, uint32_t TextSize>
void fixed_pool<T,TextSize>::store(T const &val, AddrType off)
{ write(val, off); }


template struct fixed_pool<ChunkHeader, 8>;
template struct fixed_pool<addr_wrapper, sizeof(AddrType)>;

} // namespace BDB
