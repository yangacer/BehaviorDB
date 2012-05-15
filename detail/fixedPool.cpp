#include "fixedPool.hpp"
#include "chunk.h"

namespace BDB {

template<typename T, unsigned int TextSize>
fixed_pool<T,TextSize>::fixed_pool() 
: id_(0), work_dir_(""), file_(0), fbuf_(0)
{}

template<typename T, unsigned int TextSize>
fixed_pool<T,TextSize>::fixed_pool(unsigned int id, char const* work_dir)
  : id_(0), work_dir_(""), file_(0), fbuf_(0)
{
  open(id, work_dir);
}

template<typename T, unsigned int TextSize>
fixed_pool<T,TextSize>::~fixed_pool()
{
  if(file_) fclose(file_);
  delete []fbuf_;
}

template<typename T, unsigned int TextSize>
fixed_pool<T,TextSize>::operator void const *() const
{ 
  if(!file_) return 0;
  return this;
}

template<typename T, unsigned int TextSize>
void
fixed_pool<T,TextSize>::open(unsigned int id, char const* work_dir)
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

template<typename T, unsigned int TextSize>
int fixed_pool<T,TextSize>::read(T* val, AddrType addr) const
{
  if(!*this) return -1;

  off_t loc_addr = addr;
  loc_addr *= TextSize;
  if(-1 == fseeko(file_, loc_addr, SEEK_SET))
    return -1;
  if( 0 == file_>>*val)
    return -1;
  if(ferror(file_)) return -1;
  return 0;
}

template<typename T, unsigned int TextSize>
int fixed_pool<T,TextSize>::write(T const & val, AddrType addr)
{
  if(!*this) return -1;

  off_t loc_addr = addr;
  loc_addr *= TextSize;
  if(-1 == fseeko(file_, loc_addr, SEEK_SET))
    return -1;
  if( 0 == file_<<val ) return -1;
  if(ferror(file_)) return -1;
  if(fflush(file_)) return -1;
  return 0;
}

template<typename T, unsigned int TextSize>
std::string
fixed_pool<T,TextSize>::dir() const 
{
  return work_dir_;
}

template struct fixed_pool<ChunkHeader, 8>;

} // namespace BDB
