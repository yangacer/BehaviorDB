#include "error.hpp"
#include "file_utils.hpp"
#include "poolImpl.hpp"
#include "idPool.hpp"
#include "v_iovec.hpp"
#include <boost/variant/apply_visitor.hpp>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include "addr_handle.hpp"

namespace BDB {

namespace detail {
  AddrType 
  max_addr(uint32_t chunk_size)
  {
    // consider 
    // 1. max size of idpool (bitset) 0xffffffff
    // 2. seek offset range: 2^63/chunk_size
    
    return 0;

  }
} // namespace detail

  pool::pool(pool::config const &conf, addr_eval<AddrType>& addrEval)
    : addrEval(addrEval),
    dirID(conf.dirID), 
    work_dir(conf.work_dir), trans_dir(conf.trans_dir), 
    //addrEval(conf.addrEval), 
    file_(0), idPool_(0), headerPool_(conf.dirID, conf.header_dir)
  {
    using namespace std;

    // create pool file
    char fname[256];
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
    idPool_ = new IDPool(fname, 0);

  }

  pool::~pool()
  {
    delete idPool_;
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
    
    addr_handle ah(*idPool_);

    ChunkHeader header;
    header.size = size;
    
    seek(ah.addr());

    // allow data = 0 to act as allocation
    if(0 != data && size != s_write(data, size, file_))
      throw std::runtime_error(SRC_POS);
    
    if(fflush(file_))
      throw std::runtime_error(SRC_POS);

    headerPool_.write(header, ah.addr());
    
    ah.commit();

    return ah.addr();

  }

  // off == npos represents an append write
  AddrType
  pool::write(char const* data, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;

    if(!idPool_->isAcquired(addr))
      throw std::runtime_error(SRC_POS);

    ChunkHeader loc_header;
    headerPool_.read(&loc_header, addr);

    if(size + loc_header.size > addrEval.chunk_size_estimation(dirID))
      throw internal_chunk_overflow((internal_chunk_overflow){loc_header.size});

    off = (npos == off) ? loc_header.size : off;
    
    if(npos == off){
      fseeko(file_, addr_off2tell(addr, loc_header.size), SEEK_SET);
      if(size != s_write(data,size,file_) || fflush(file_))
       throw std::runtime_error(SRC_POS);
      loc_header.size += size;
      headerPool_.write(loc_header, addr);
    }else{
      addr = merge_move(data, size, addr, off, this, &loc_header);
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
    ChunkHeader header;
    addr_handle ah(*idPool_);
    
    header.size = 
      writevv(vv, len, file_, 
            addr_off2tell(ah.addr(),0) );
     
    headerPool_.write(header, ah.addr());
    ah.commit();
    
    return ah.addr();
  }

  AddrType
  pool::replace(char const *data, uint32_t size, AddrType addr)
  {
    using namespace detail;

    if(!idPool_->isAcquired(addr))
      throw invalid_addr();

    ChunkHeader loc_header, new_header;
    headerPool_.read(&loc_header, addr);

    assert(size <= addrEval.chunk_size_estimation(dirID));
    
    new_header.size = size;

    seek(addr, 0);

    if(size != s_write(data, size, file_) ||
       0 != fflush(file_))
    {
      idPool_->Release(addr);
      idPool_->Commit(addr);
      throw std::runtime_error(SRC_POS);
    }

    // update header 
    try{
      headerPool_.write(new_header, addr);
    }catch(std::runtime_error const &){
      idPool_->Release(addr);
      idPool_->Commit(addr);
      throw std::runtime_error(SRC_POS);
    }

    return addr;
  }


  uint32_t
  pool::read(char* buffer, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;

    if(!idPool_->isAcquired(addr))
      throw std::runtime_error(SRC_POS);

    ChunkHeader loc_header;
    headerPool_.read(&loc_header, addr);

    if(off > loc_header.size)
      return 0;

    seek(addr, off);

    uint32_t toRead = (size > loc_header.size - off) ? 
      loc_header.size - off 
      : size;

    uint32_t rcnt = 0;

    if(toRead != (rcnt = s_read(buffer, toRead, file_)))
      throw std::runtime_error(SRC_POS);

    return toRead;
  }

  uint32_t
  pool::read(std::string *buffer, uint32_t max, AddrType addr, uint32_t off)
  {
    if(!buffer) return 0;
    if(buffer->size()) buffer->clear();
    uint32_t readCnt(0), total(0);
    while(0 < (readCnt = read(mig_buf_, MIGBUF_SIZ, addr, off))){
      if((uint32_t)-1 == readCnt) return -1;
      if(total + readCnt > max) break;
      buffer->append(mig_buf_, readCnt);
      off += readCnt;
      total += readCnt;
    }
    return total;
  }

  AddrType
  pool::merge_copy(
    char const* data, 
    uint32_t size, 
    AddrType src_addr, 
    uint32_t off, 
    pool *dest_pool,
    ChunkHeader const* header)
  {
    ChunkHeader loc_header;
    if(header)
      loc_header = *header;
    else
      headerPool_.read(&loc_header, src_addr);

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
      vv[1].size = loc_header.size;
      loc_addr = dest_pool->write(vv, 2);
    }else if(loc_header.size == off){ // append
      fs.fp = file_;
      fs.off = addr_off2tell(src_addr, 0);
      vv[0].data = fs;
      vv[0].size = loc_header.size;
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
      vv[2].size = loc_header.size - off;
      loc_addr = dest_pool->write(vv, 3);
    }
    
    return loc_addr;
  }

  AddrType
  pool::merge_move(char const*data, 
                   uint32_t size, 
                   AddrType src_addr, 
                   uint32_t off, 
                   pool *dest_pool, 
                   ChunkHeader const *header)
  {

    AddrType loc_addr = 
      merge_copy(data, size, src_addr, off, dest_pool, header);

    idPool_->Release(src_addr);
    idPool_->Commit(src_addr);

    return loc_addr;
  }

  uint32_t
  pool::free(AddrType addr)
  { 
    if(!idPool_->isAcquired(addr))
      throw invalid_addr();

    idPool_->Release(addr);
    idPool_->Commit(addr);
    
    return 0;
  }

  uint32_t
  pool::erase(AddrType addr, uint32_t off, uint32_t size)
  { 
    using namespace detail;

    if(!idPool_->isAcquired(addr))
      throw invalid_addr();

    ChunkHeader header;
    headerPool_.read(&header, addr);

    if(off > header.size) return header.size;

    // overflow check
    size = (off + size > off) ?  
      (off + size > header.size) ? header.size - off : size
      : header.size - off;

    uint32_t toRead = header.size - size - off;
    uint32_t readCnt, loopOff(0);

    header.size -= size;
    headerPool_.write(header, addr);

    while(toRead > 0){
      readCnt = (toRead > MIGBUF_SIZ) ? MIGBUF_SIZ : toRead;
      seek(addr, off + size + loopOff);

      if(readCnt != s_read(mig_buf_, readCnt, file_))
        throw std::runtime_error(SRC_POS);

      seek(addr, off + loopOff);
      
      if(readCnt != s_write(mig_buf_, readCnt, file_) ||
         0 != fflush(file_))
        throw std::runtime_error(SRC_POS);

      loopOff += readCnt;
      toRead -= readCnt;
    }

    return header.size;
  }

  uint32_t
  pool::overwrite(char const* data, uint32_t size, AddrType addr, uint32_t off)
  {
    using namespace detail;
    
    if(!idPool_->isAcquired(addr))
      throw invalid_addr();

    seek(addr, off);

    if(size != s_write(data, size, file_) ||
       fflush(file_) )
      throw std::runtime_error(SRC_POS);

    return size;
  }

  int
  pool::head(ChunkHeader *header, AddrType addr) const
  { 
    headerPool_.read(header, addr);
    return 0;
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
    assert(0 != *this && "pool is not proper initiated");

    std::pair<int, int> rt(0,0);
    if(!err_.empty()){
      rt = err_.front();
      err_.pop_front();
    }
    return rt;

  }

  void
  pool::pine(AddrType addr)
  { idPool_->Lock(addr); }

  void
  pool::unpine(AddrType addr)
  { idPool_->Unlock(addr); }

  bool
  pool::is_pinned(AddrType addr)
  { return idPool_->isLocked(addr); }

} // end of BDB namespace
