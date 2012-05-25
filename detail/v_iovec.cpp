#include "common.hpp"
#include "v_iovec.hpp"

namespace BDB {

  size_t
  write_viov::operator()(file_src &fsrc)
  {

    if(-1 == fseeko(fsrc.fp, fsrc.off, SEEK_SET))
      return 0;

    size_t toRead = size;
    size_t readCnt, loopOff(0);
    while(toRead){
      readCnt = (bsize > toRead) ? toRead : bsize;
      fseeko(fsrc.fp, fsrc.off + loopOff, SEEK_SET);
      if(readCnt != fread(buf, 1, readCnt, fsrc.fp))
        return 0;
      fseeko(dest, dest_pos + loopOff, SEEK_SET);
      //write
      if(readCnt != fwrite(buf, 1, readCnt, dest) || fflush(dest))
        return 0;
      loopOff += readCnt;
      toRead -= readCnt;
    }
    return size;
  }
  
  size_t
  write_viov::operator()(char const* str)
  {
    fseeko(dest, dest_pos, SEEK_SET);
    if(size != fwrite(str, 1, size, dest) || fflush(dest))
      return 0;
    return size;
  }
  
  size_t
  write_viov::operator()(no_data_src & ndsrc)
  {
    fseeko(dest, dest_pos, SEEK_SET);
    return size;
  }

} // end of namespace BDB
