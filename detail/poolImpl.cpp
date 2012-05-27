#include "error.hpp"
#include "file_utils.hpp"
#include "poolImpl.hpp"
#include "id_pool.hpp"
#include "id_handle.hpp"
#include "v_iovec.hpp"
#include <boost/variant/apply_visitor.hpp>
#include <cassert>
#include <cstdio>
#include <stdexcept>

namespace BDB {
  
  pool::pool(pool::config const &conf, addr_eval<AddrType>& addrEval)
    : addrEval(addrEval),
    dirID(conf.dirID), 
    work_dir(conf.work_dir), trans_dir(conf.trans_dir), 
    //addrEval(conf.addrEval), 
    file_(0), idpool_(0)
  {
    using namespace std;

    // create pool file
    char fname[256] = {};

    if(work_dir.size() > 256) 
      throw length_error("pool: length of pool_dir string is too long");


    sprintf(fname, "%s%04x.pool", work_dir.c_str(), dirID);
    if(0 == (file_ = fopen(fname, "r+b"))){
      if(0 == (file_ = fopen(fname, "w+b"))){
        string msg("pool: Unable to create pool file ");
        msg += fname;
        throw invalid_argument(msg.c_str());
      }
    }

    mig_buf_ = new char[MIGBUF_SIZ];
    file_buf_ = new char[MIGBUF_SIZ];

    if(0 != setvbuf(file_, file_buf_, _IOFBF, MIGBUF_SIZ))
      throw runtime_error("pool: setvbuf to pool file failed");

    // setup idPool
    sprintf(fname, "%s%04x.tran", trans_dir.c_str(), dirID);
    
    // address
    idpool_ = new idpool_t(dirID, trans_dir.c_str(), 0, npos, dynamic);

  }

  pool::~pool()
  {
    delete idpool_;
    delete [] file_buf_;
    delete [] mig_buf_;
    fclose(file_);
  }

  pool::operator void const*() const
  { 
    //if(!this || !addrEval.is_init() || !idPool_)
    //  return 0;
    return this;
  }

  AddrType
  pool::write(char const* data, uint32_t size)
  {
    using namespace detail;
   
    id_handle_t hdl(ACQUIRE_AUTO, *idpool_);

    hdl.value().size = size;
      
    seek(hdl.addr());

    // allow data = 0 to act as allocation
    if(0 != data && size != s_write(data, size, file_))
      throw std::runtime_error(SRC_POS);
    
    if(fflush(file_))
      throw std::runtime_error(SRC_POS);

    hdl.commit();

    return hdl.addr();
  }

  // off == npos represents an append write
  AddrType
  pool::write(char const* data, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;
    
    id_handle_t hdl(MODIFY, *idpool_, addr);

    ChunkHeader &loc_header(hdl.value());
    
    if(size + loc_header.size > addrEval.chunk_size_estimation(dirID))
      throw internal_chunk_overflow((internal_chunk_overflow){loc_header.size});

    off = (npos == off) ? loc_header.size : off;
    
    if(npos == off){
      fseeko(file_, addr_off2tell(addr, loc_header.size), SEEK_SET);
      if(size != s_write(data,size,file_) || fflush(file_))
       throw std::runtime_error(SRC_POS);
      loc_header.size += size;
      hdl.commit();
    }else{
      addr = merge_move(data, size, addr, off, this);
    }
    return addr;

    /* In-place merge maybe save some cost, though.
     * I may re-enable this if it's a critical bottleneck
     *
     * XXX See commit c8754a for old implementation
     */
  }

  AddrType
  pool::write(viov* vv, uint32_t len)
  {
    using namespace detail;
    id_handle_t hdl(ACQUIRE_AUTO, *idpool_);
    
    hdl.value().size = 
      writevv(vv, len, file_, 
            addr_off2tell(hdl.addr(),0) );
     
    hdl.commit();
    
    return hdl.addr();
  }

  AddrType
  pool::replace(char const *data, uint32_t size, AddrType addr)
  {
    using namespace detail;
    
    assert(size <= addrEval.chunk_size_estimation(dirID));

    id_handle_t hdl(MODIFY, *idpool_, addr);
    
    hdl.value().size = size;
    seek(addr, 0);

    if(size != s_write(data, size, file_) ||
       0 != fflush(file_))
      throw data_currupted(
        (data_currupted){addr} );
    
    hdl.commit();

    return addr;
  }

  uint32_t
  pool::read(char* buffer, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;
    
    id_handle_t hdl(READONLY, *idpool_, addr);
    uint32_t orig_size = hdl.const_value().size;

    if(off > orig_size)
      return 0;

    seek(addr, off);

    uint32_t toRead = (size > orig_size - off) ? 
      orig_size - off 
      : orig_size;

    return s_read(buffer, toRead, file_);
  }

  uint32_t
  pool::read(std::string *buffer, uint32_t max, AddrType addr, uint32_t off)
  {
    if(!buffer) return 0;
    if(buffer->size()) buffer->clear();

    uint32_t readCnt(0), total(0);
    while(0 < (readCnt = read(mig_buf_, MIGBUF_SIZ, addr, off))){
      total += readCnt;
      if(total >= max) break;
      buffer->append(mig_buf_, readCnt);
      off += readCnt;
    }
    return total;
  }

  AddrType
  pool::merge_copy(
    char const* data, 
    uint32_t size, 
    AddrType src_addr, 
    uint32_t off, 
    pool *dest_pool)
  {
    using namespace detail;
    id_handle_t hdl(READONLY, *idpool_, src_addr);
    uint32_t orig_size = hdl.const_value().size;
    
    // TODO make it shorter
    viov vv[3];
    file_src fs;
    blank_src blank;
    AddrType loc_addr;
    if(0 == off){ // prepend
      if(0 == data)
        vv[0].data = blank;
      else
        vv[0].data = data;
      vv[0].size = size;
      fs.fp = file_;
      fs.off = addr_off2tell(src_addr, 0);
      vv[1].data = fs;
      vv[1].size = orig_size;
      loc_addr = dest_pool->write(vv, 2);
    }else if(orig_size == off){ // append
      fs.fp = file_;
      fs.off = addr_off2tell(src_addr, 0);
      vv[0].data = fs;
      vv[0].size = orig_size;
      if(0 == data)
        vv[1].data = blank;
      else
        vv[1].data = data;
      vv[1].size = size;
      loc_addr = dest_pool->write(vv, 2);
    }else{ // insert
      fs.fp = file_;
      fs.off = addr_off2tell(src_addr, 0);
      vv[0].data = fs;
      vv[0].size = off;
      if(0 == data)
        vv[1].data = blank;
      else
        vv[1].data = data;
      vv[1].size = size;
      fs.off += off;
      vv[2].data = fs;
      vv[2].size = orig_size - off;
      loc_addr = dest_pool->write(vv, 3);
    }
    
    return loc_addr;
  }

  AddrType
  pool::merge_move(
    char const*data, 
    uint32_t size, 
    AddrType src_addr, 
    uint32_t off, 
    pool *dest_pool)
  {
    using namespace detail;
    
    id_handle_t hdl(RELEASE, *idpool_, src_addr);

    AddrType loc_addr = 
      merge_copy(data, size, src_addr, off, dest_pool);
    
    hdl.commit();

    return loc_addr;
  }

  uint32_t
  pool::free(AddrType addr)
  { 
    using namespace detail;
    id_handle_t hdl(RELEASE, *idpool_, addr);
    hdl.commit();
    return 0;
  }

  uint32_t
  pool::erase(AddrType addr, uint32_t off, uint32_t size)
  { 
    using namespace detail;
    
    id_handle_t hdl(MODIFY, *idpool_, addr);
    uint32_t orig_size = hdl.value().size;

    if(off > orig_size) return orig_size;

    // overflow check
    size = (off + size > off) ?  
      (off + size > orig_size) ? orig_size - off : size
      : orig_size - off;

    uint32_t toRead = orig_size - size - off;
    uint32_t readCnt, loopOff(0);

    while(toRead > 0){
      readCnt = (toRead > MIGBUF_SIZ) ? MIGBUF_SIZ : toRead;
      seek(addr, off + size + loopOff);
      if(readCnt != s_read(mig_buf_, readCnt, file_)){
        if(loopOff == 0)
          return orig_size;
        else
          throw data_currupted((data_currupted){addr});
      }
      seek(addr, off + loopOff);
      if(readCnt != s_write(mig_buf_, readCnt, file_) ||
         0 != fflush(file_))
      {
        throw data_currupted((data_currupted){addr});
      }
      loopOff += readCnt;
      toRead -= readCnt;
    }
    
    hdl.value().size -= size;
    hdl.commit();

    return hdl.value().size;
  }

  uint32_t
  pool::overwrite(char const* data, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;
    
    if(!idpool_->isAcquired(addr))
      throw invalid_addr();

    seek(addr, off);

    if(size != s_write(data, size, file_) ||
       fflush(file_) )
      throw data_currupted((data_currupted){addr});

    return size;
  }

  off_t
  pool::seek(AddrType addr, uint32_t off)
  {
    off_t pos = addr;
    pos *= addrEval.chunk_size_estimation(dirID);
    pos += off;
    fseeko(file_, pos, SEEK_SET);
    return pos;
  }


  off_t
  pool::addr_off2tell(AddrType addr, uint32_t off) const
  {
    off_t pos = addr;
    pos *= addrEval.chunk_size_estimation(dirID);
    pos += off;
    return pos;
  }

  void
  pool::on_error(int errcode, int line)
  {
    // TODO: lock for mutli proc/thread
    err_.push_back(std::make_pair<int, int>(errcode, line));	
    // unlock
  }

  std::pair<int, int>
  pool::get_error()
  {
    std::pair<int, int> rt(0,0);
    if(!err_.empty()){
      rt = err_.front();
      err_.pop_front();
    }
    return rt;
  }

  void
  pool::pine(AddrType addr)
  { idpool_->Lock(addr); }

  void
  pool::unpine(AddrType addr)
  { idpool_->Unlock(addr); }

  bool
  pool::is_pinned(AddrType addr)
  { return idpool_->isLocked(addr); }

} // end of BDB namespace
