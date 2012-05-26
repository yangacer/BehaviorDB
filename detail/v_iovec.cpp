#include "common.hpp"
#include "v_iovec.hpp"
#include "file_utils.hpp"
#include "error.hpp"
#include <stdexcept>
namespace BDB {
  
  using namespace detail;

  struct write_viov : public boost::static_visitor<size_t>
  {
    size_t 
    operator()(char const* str);
    
    size_t
    operator()(file_src & psrc);
    
    size_t
    operator()(blank_src &);
    
    FILE* dest;
    off_t dest_pos;
    size_t size;
  };
  
  size_t
  write_viov::operator()(file_src &fsrc)
  {
    // TODO Use pool allocator
    char buf[1023+1];
    size_t const bsize = 1023;
    
    size_t toRead = size;
    size_t readCnt, loopOff(0);
    while(toRead){
      readCnt = (bsize > toRead) ? toRead : bsize;
      fseeko(fsrc.fp, fsrc.off + loopOff, SEEK_SET);
      if(readCnt != s_read(buf, readCnt, fsrc.fp))
        throw std::runtime_error(SRC_POS);
      
      fseeko(dest, dest_pos + loopOff, SEEK_SET);
      //write
      if(readCnt != s_write(buf, readCnt, dest))
        throw std::runtime_error(SRC_POS);
        
      loopOff += readCnt;
      toRead -= readCnt;
    }
    dest_pos += size;
    return size;
  }
  
  size_t
  write_viov::operator()(char const* str)
  {
    fseeko(dest, dest_pos, SEEK_SET);
    if(size != s_write(str, size, dest))
      throw std::runtime_error(SRC_POS);
    dest_pos += size;
    return size;
  }
  
  size_t
  write_viov::operator()(blank_src &)
  {
    fseeko(dest, dest_pos, SEEK_SET);
    dest_pos += size;
    return size;
  }

  size_t writevv(viov *vv, size_t len, FILE* dest, size_t off)
  {
    using boost::apply_visitor;

    size_t rt(0);

    write_viov wv;
    wv.dest = dest;
    wv.dest_pos = off;

    for(size_t i =0;i<len;++i){
      wv.size = vv[i].size;
      apply_visitor(wv, vv[i].data);
      rt += vv[i].size;
    }

    if(fflush(dest))
      throw std::runtime_error(SRC_POS);
    return rt;
  }

} // end of namespace BDB
