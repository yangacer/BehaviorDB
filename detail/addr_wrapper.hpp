#ifndef BDB_ADDR_WRAP_HPP_
#define BDB_ADDR_WRAP_HPP_

#include <cstdio>
#include <stdexcept>
#include "common.hpp"
namespace BDB {

struct addr_wrapper
{
  AddrType addr;
  addr_wrapper() : addr(0) {}
  addr_wrapper(AddrType addr) : addr(addr) {}
  operator AddrType() const { return addr; }
};

inline FILE* operator<<(FILE* fp, addr_wrapper const & a)
{ 
  size_t rt = fwrite((void*)&a.addr, sizeof(AddrType), 1, fp);
  fflush(fp);
  if(rt != sizeof(AddrType) && ferror(fp))
    throw std::runtime_error("write addr failed");
  return fp;
}

inline FILE* operator>>(FILE* fp, addr_wrapper & a)
{ 
  size_t rt = fread(&a.addr, sizeof(AddrType), 1, fp);
  if(rt != sizeof(AddrType) && ferror(fp))
    throw std::runtime_error("read addr failed");
  return fp;
}
inline std::ostream & operator<<(std::ostream & fp, addr_wrapper const & a)
{ 
  fp.write((char const*)&a.addr, sizeof(AddrType));
  fp.flush();
  if(!fp)
    throw std::runtime_error("write addr failed");
  return fp;
}

inline std::istream & operator>>(std::istream & fp, addr_wrapper & a)
{ 
  fp.read((char *)&a.addr, sizeof(AddrType));
  if(!fp && !fp.eof())
    throw std::runtime_error("read addr failed");
  return fp;
}
} // namespace BDB

#endif
