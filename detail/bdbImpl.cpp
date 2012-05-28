#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "error.hpp"
#include "id_pool.hpp"
#include "id_handle.hpp"
#include "fixedPool.hpp"
#include "addr_iter.hpp"
#include "stat.hpp"
//#include "stream_state.hpp"
#include <cassert>
#include <stdexcept>
#include <ios>
#include <sstream>

namespace BDB {
  
  BDBImpl::BDBImpl(Config const & conf)
  : pools_(0), err_log_(0), acc_log_(0), global_id_(0)
  {
    conf.validate();
    init_(conf); 
  }
  
  BDBImpl::~BDBImpl()
  {
    delete global_id_;

    if(acc_log_) fclose(acc_log_);
    if(err_log_) fclose(err_log_);

    if(!pools_) return;
    for(unsigned int i =0; i<addrEval.dir_count(); ++i)
      pools_[i].~pool();
    free(pools_);
  }
  
  void
  BDBImpl::init_(Config const & conf)
  {
    addrEval.init(
      conf.addr_prefix_len, 
      conf.min_size, 
      conf.cse_func, 
      conf.ct_func);

    // initial pools
    pool::config pcfg;
    pcfg.work_dir = (*conf.pool_dir) ? conf.pool_dir : conf.root_dir;
    pcfg.trans_dir =(*conf.trans_dir) ?  conf.trans_dir : conf.root_dir;
    pcfg.header_dir = (*conf.header_dir) ? conf.header_dir : conf.root_dir;

    pools_ = (pool*)malloc(sizeof(pool) * addrEval.dir_count());
    for(unsigned int i =0; i<addrEval.dir_count(); ++i){
      pcfg.dirID = i;
      new (&pools_[i]) pool(pcfg, addrEval); 
    }

    // init logs
    char fname[256] = {};
    if(conf.log_dir){
      char const* log_dir = (*conf.log_dir) ? conf.log_dir : conf.root_dir;
      if(strlen(log_dir) > 256)
        throw std::length_error("length of pool_dir string is too long\n");

      sprintf(fname, "%serror.log", log_dir);
      if(0 == (err_log_ = fopen(fname, "ab")))
        throw std::runtime_error("create error log file failed\n");
    
      if(0 != setvbuf(err_log_, err_log_buf_, _IOLBF, 256))
        throw std::runtime_error("setvbuf to log file failed\n");
      
      sprintf(fname, "%saccess.log", log_dir);
      if(0 == (acc_log_ = fopen(fname, "ab")))
        throw std::runtime_error("create access log file failed\n");
    
      if(0 != setvbuf(acc_log_, acc_log_buf_, _IOFBF, 4096))
        throw std::runtime_error("setvbuf to log file failed\n");
    }

    // init IDValPool
    sprintf(fname, "%sgid_", conf.root_dir);
    global_id_ = new idpool_t(0, fname, conf.beg, npos, dynamic);
  }
  
  AddrType
  BDBImpl::put(char const *data, uint32_t size)
  {
    if(!global_id_->avail()) 
      throw addr_overflow();
    
    id_handle_t hdl(detail::ACQUIRE_AUTO, *global_id_);
    hdl.value() = write_pool(data, size);
    hdl.commit();
    
    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\n", "put", size);

    return hdl.addr();
  }


  AddrType
  BDBImpl::put(char const* data, uint32_t size, AddrType addr, uint32_t off)
  {
    try{
      id_handle_t hdl(detail::ACQUIRE_SPEC, *global_id_, addr);
      hdl.value() = write_pool(data, size);
      hdl.commit();
      if(acc_log_)
        fprintf(acc_log_, "%-12s\t%08x\t%08x\n", 
                "put-spec", size, addr);
    }catch(BDB::invalid_addr const &ia){

      id_handle_t hdl(detail::MODIFY, *global_id_, addr);

      unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
      AddrType loc_addr = addrEval.local_addr(hdl.const_value());
      AddrType nwe_addr;
      try{
        // no pool-to-pool migration 
        loc_addr = pools_[dir].write(data, size, loc_addr, off);
        hdl.value() = addrEval.global_addr(dir, loc_addr);
        hdl.commit();
        if(acc_log_)
          fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
                  "insert", size, addr, off);
      }catch(internal_chunk_overflow const &co){
        // migration
        unsigned int next_dir = 
          addrEval.directory(size + co.current_size);
        AddrType next_loc_addr;

        if((unsigned int)-1 == next_dir)
          throw chunk_overflow();

        if(npos == off)
          off = co.current_size;

        while(next_dir < addrEval.dir_count()){
          try{
            next_loc_addr = 
              pools_[dir].merge_move(
                data, size, loc_addr, off,
                &pools_[next_dir]);
          }catch(addr_overflow const &){
            next_dir++;
            continue;
          }
          break;
        }
        if( next_dir >= addrEval.dir_count())
          throw addr_overflow();

        hdl.value() = addrEval.global_addr(next_dir, next_loc_addr);
        hdl.commit();

        if(acc_log_)
          fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
                  "insert", size, addr, off);
      }
    }
    return addr;
  }
  
  AddrType
  BDBImpl::update(char const *data, uint32_t size, AddrType addr)
  {
    id_handle_t hdl(detail::MODIFY, *global_id_, addr);
    AddrType internal_addr;

    internal_addr = global_id_->Find(addr);

    unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
    AddrType loc_addr = addrEval.local_addr(hdl.const_value());

    // check size
    if( !addrEval.capacity_test(dir, size) ){

      unsigned int old_dir = dir;
      AddrType old_loc_addr = loc_addr;
      AddrType new_internal_addr = write_pool(data, size);
 
      hdl.value() = new_internal_addr;
      hdl.commit();

      pools_[old_dir].free(old_loc_addr);

      if(acc_log_) 
        fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "update_put", size, addr);

    }else{
      loc_addr = pools_[dir].replace(data, size, loc_addr);

      if(acc_log_) 
        fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "update", size, addr);
    }

    return addr;
  }

  uint32_t
  BDBImpl::get(char *output, uint32_t size, AddrType addr, uint32_t off)
  {
    id_handle_t hdl(detail::READONLY, *global_id_, addr);

    uint32_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
    AddrType loc_addr = addrEval.local_addr(hdl.const_value());
    
    rt = pools_[dir].read(output, size, loc_addr, off);
    
    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "get", size, addr, off);

    return rt;
  }
  
  uint32_t
  BDBImpl::get(std::string *output, uint32_t max, AddrType addr, uint32_t off)
  {
    id_handle_t hdl(detail::READONLY, *global_id_, addr);

    uint32_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
    AddrType loc_addr = addrEval.local_addr(hdl.const_value());
    
    rt = pools_[dir].read(output, max, loc_addr, off);

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "string_get", max, addr, off);

    return rt;
  }

  uint32_t
  BDBImpl::del(AddrType addr)
  {
    id_handle_t hdl(detail::READONLY, *global_id_, addr);
   
    unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
    AddrType loc_addr = addrEval.local_addr(hdl.const_value());
    
    pools_[dir].free(loc_addr);

    {
      id_handle_t hdl(detail::RELEASE, *global_id_, addr);
      hdl.commit();
    }

    if(acc_log_)
      fprintf(acc_log_, "%-12s\t%08x\n", "del", addr);

    return 0;
  }

  uint32_t
  BDBImpl::del(AddrType addr, uint32_t off, uint32_t size)
  {
    
    id_handle_t hdl(detail::MODIFY, *global_id_, addr);

    unsigned int dir = addrEval.addr_to_dir(hdl.const_value());
    AddrType loc_addr = addrEval.local_addr(hdl.const_value());
    uint32_t nsize;

    nsize = pools_[dir].erase(loc_addr, off, size);
    hdl.commit();

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "partial_del", addr, off, size);
    
    return nsize;
  }
  
  /*
  bool
  BDBImpl::stream_error(stream_state const* state)
  { return state->error; }

  stream_state const*
  BDBImpl::ostream(uint32_t stream_size)
  {
    // assert(0 != *this && "BDBImpl is not proper initiated");
    
    if(!global_id_->avail()){
      error(ADDRESS_OVERFLOW, __LINE__);
      return 0;
    }
    
    unsigned int dir = addrEval.directory(stream_size);
    
    AddrType inter_addr = write_pool(NULL, stream_size);

    if(-1 == inter_addr) return NULL;

    if(acc_log_) fprintf(acc_log_, "%-12s\t%08x\n", "ostream", stream_size);
    
    stream_state *rt = stream_state_pool_.malloc();
    if(0 == rt) return 0;

    rt->read_write = stream_state::WRT;
    rt->existed = false;
    rt->ext_addr = -1;
    rt->error = false;
    rt->inter_dest_addr = inter_addr;
    rt->offset = 0;
    rt->size = stream_size;
    rt->used = 0;

    return rt;
  }
  
  // TODO suuport replace mode
  stream_state const*
  BDBImpl::ostream(uint32_t stream_size, AddrType addr, uint32_t off)
  {
    if( global_id_->isLocked(addr) )
      return 0;

    global_id_->Lock(addr);

    if( !global_id_->isAcquired(addr)){
      // TODO: need to support this   
      // UTILIZE ext_addr to save acquiring addr
      stream_state * ss = 
        const_cast<stream_state*>(ostream(stream_size));
      if(ss) ss->ext_addr = addr;
      return ss;
    }


    AddrType internal_addr;
    internal_addr = global_id_->Find(addr);

    unsigned int dir = addrEval.addr_to_dir(internal_addr);
    AddrType loc_addr = addrEval.local_addr(internal_addr);
    
    ChunkHeader header;
    if(-1 == pools_[dir].head(&header, loc_addr)){
      error(dir);
      global_id_->Unlock(addr);
      return 0;
    }
    
    unsigned int next_dir = 
      addrEval.directory(stream_size + header.size);

    if(-1 == next_dir){
      error(DATA_TOO_BIG, __LINE__);
      global_id_->Unlock(addr);
      return 0;
    }
    
    off = (npos == off) ? header.size : off;
    
    // TODO: append optimization (no copy)
    AddrType next_loc_addr =
      pools_[dir].merge_copy( 
        0, stream_size, loc_addr, off,
        &pools_[next_dir], &header); 
    
    if(-1 == next_loc_addr){
      error(next_dir);
      global_id_->Unlock(addr);
      return 0;
    }
    
    if(acc_log_) fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
      "ostream_ins", stream_size, addr, off);

    stream_state *rt = stream_state_pool_.malloc();
    if(0 == rt) return 0;
    rt->read_write = stream_state::WRT;
    rt->existed = true;
    rt->error = false;
    rt->ext_addr = addr;
    rt->inter_src_addr = internal_addr;
    rt->inter_dest_addr = addrEval.global_addr(
        next_dir, next_loc_addr);
    rt->offset = off;
    rt->size = stream_size;
    rt->used = 0;

    return rt;
  }
  
  stream_state const*
  BDBImpl::istream(uint32_t stream_size, AddrType addr, uint32_t off)
  {
    /// TODO Consider allow reader when a chunk is been written
    if(!global_id_->isAcquired(addr) || global_id_->isLocked(addr))
      return 0;

    AddrType inter_addr = global_id_->Find(addr); 

    // register to in_reading hash table
    AddrCntCont::iterator iter;
    if(in_reading_.end() == (iter = in_reading_.find(inter_addr)))
      in_reading_[inter_addr] = 1;
    else
      iter->second++;

    stream_state *rt = stream_state_pool_.malloc();
    if(0 == rt) return 0;

    rt->read_write = stream_state::READ;
    rt->existed = true;
    rt->error = false;
    rt->ext_addr = addr;
    rt->inter_src_addr = inter_addr;
    rt->offset = off;
    rt->size = stream_size;
    rt->used = 0;

    return rt;
  }

  stream_state const*
  BDBImpl::stream_write(stream_state const* state, char const* data, uint32_t size)
  {
    stream_state *ss = const_cast<stream_state*>(state);
    if(ss->size - ss->used < size){
      ss->error = true;
      return ss;
    }

    unsigned int dir = addrEval.addr_to_dir(ss->inter_dest_addr);
    AddrType loc_addr = addrEval.local_addr(ss->inter_dest_addr);

    if(size != pools_[dir].overwrite(
      data, size, loc_addr, ss->offset + ss->used) )
    {
      error(dir);
      ss->error = true;
      return ss;
    }
    
    ss->used += size;
    
    return ss;
  }
  
  stream_state const*
  BDBImpl::stream_read(stream_state const* state, char *output, uint32_t size)
  {
    // TODO consistency checking
    stream_state *ss = const_cast<stream_state*>(state);

    unsigned int dir = addrEval.addr_to_dir(ss->inter_src_addr);
    AddrType loc_addr = addrEval.local_addr(ss->inter_src_addr);

    uint32_t toRead = (ss->size - ss->used < size) ?
      ss->size - ss->used : size;
    
    if(toRead != pools_[dir].read(output, size, loc_addr,
      ss->offset + ss->used))
    {
      error(dir);
      ss->error = true;
    }
    ss->used += toRead;
    return ss;

  }

  AddrType
  BDBImpl::stream_finish(stream_state const* state)
  {
    if(state->error){
      stream_abort(state);
      return -1;
    }

    stream_state *ss = const_cast<stream_state*>(state);
    AddrType rt;
    
    unsigned int dir = 
      addrEval.addr_to_dir(ss->inter_src_addr);
    AddrType loc_addr = 
      addrEval.local_addr(ss->inter_src_addr);

    // read mode
    if(stream_state::READ == ss->read_write){
      // unpine if the inter_dest addr is pinned 
      // and a reader is the last one
      AddrCntCont::iterator iter = 
        in_reading_.find(ss->inter_src_addr);

      assert(in_reading_.end() != iter && 
        "unexpected address");

      iter->second--;
      if(iter->second == 0){ // the last reader
        if(pools_[dir].is_pinned(loc_addr)){
          pools_[dir].unpine(loc_addr);
          pools_[dir].free(loc_addr);
        }
        in_reading_.erase(iter);
      }
      
      rt = ss->ext_addr;
      stream_state_pool_.free(ss);
      return rt;
    }
    
    // write mode
    if(ss->used == ss->size){
      if(ss->existed){
        
        // if anyone is reading the chunk
        // do pine instead of free
        AddrCntCont::iterator iter = 
          in_reading_.find(ss->inter_src_addr);
        if(in_reading_.end() != iter)
          pools_[dir].pine(loc_addr);
        else
          pools_[dir].free(loc_addr);

        global_id_->Update(ss->ext_addr, ss->inter_dest_addr);
        
        if(!global_id_->Commit(ss->ext_addr)){
          global_id_->Update(ss->ext_addr, ss->inter_src_addr);
          error(COMMIT_FAILURE, __LINE__);  
          return -1;
        }
      }else {
        if(-1 == ss->ext_addr){
          if(-1 == (ss->ext_addr = 
                global_id_->Acquire(ss->inter_dest_addr)))
          {
            error(SYSTEM_ERROR, __LINE__);
            stream_abort(state);
            return -1;
          }
        }else {
          if(-1 == global_id_->Acquire(ss->ext_addr, ss->inter_dest_addr))
          {
             error(SYSTEM_ERROR, __LINE__);
            stream_abort(state);
            return -1;
 
          }
          global_id_->Unlock(ss->ext_addr);
        }
        if(!global_id_->Commit(ss->ext_addr)){
          global_id_->Release(ss->ext_addr);
          error(COMMIT_FAILURE, __LINE__);
          return -1;
        }
      } 
      rt = ss->ext_addr;
      stream_state_pool_.free(ss);
    }else { //incomplete buffer
      stream_abort(state);
      rt = -1;
    }
    return rt;
  }
  
  uint32_t
  BDBImpl::stream_pause(stream_state const* state)
  {
    uint32_t rt = reinterpret_cast<uint32_t>(state);
    rt ^= 0xDEA3;

    assert(enc_stream_state_.end() == enc_stream_state_.find(rt));

    enc_stream_state_.insert(rt);

    return rt;
  }
  
  stream_state const*
  BDBImpl::stream_resume(uint32_t encrypt_handle)
  {
    if(enc_stream_state_.end() == enc_stream_state_.find(encrypt_handle) )
      return 0;
    enc_stream_state_.erase(encrypt_handle);
    encrypt_handle ^= 0xDEA3;
    return reinterpret_cast<stream_state const*>(encrypt_handle);
  }

  void
  BDBImpl::stream_expire(uint32_t encrypt_handle)
  {
    assert(enc_stream_state_.end() == 
      enc_stream_state_.find(encrypt_handle) );

    stream_state const* state = stream_resume(encrypt_handle);
    enc_stream_state_.erase(encrypt_handle);
    stream_abort(state);
  }

  void
  BDBImpl::stream_abort(stream_state const* state)
  {
    stream_state *ss = const_cast<stream_state*>(state);
    
    if(stream_state::READ == ss->read_write){
      unsigned int dir = 
        addrEval.addr_to_dir(ss->inter_src_addr);
      AddrType loc_addr = 
        addrEval.local_addr(ss->inter_src_addr);
      
      // unpine if the inter_dest addr is pinned 
      // and a reader is the last one
      AddrCntCont::iterator iter = 
        in_reading_.find(ss->inter_src_addr);

      assert(in_reading_.end() != iter && 
        "unexpected address");

      iter->second--;
      
      if(iter->second == 0){ // the last reader
        if(pools_[dir].is_pinned(loc_addr)){
          pools_[dir].unpine(loc_addr);
          pools_[dir].free(loc_addr);
        }
        in_reading_.erase(iter);
      }

      stream_state_pool_.free(ss);
      return;
    }
  
    // write mode
    unsigned int dir = addrEval.addr_to_dir(ss->inter_dest_addr);
    AddrType loc_addr = addrEval.local_addr(ss->inter_dest_addr);
    
    if(-1 == pools_[dir].free(loc_addr))
      error(dir);
    
    if(-1 != ss->ext_addr) global_id_->Unlock(ss->ext_addr);

    stream_state_pool_.free(ss);
  }
  */

  AddrIterator
  BDBImpl::begin() const
  {
    AddrType first_used = global_id_->begin();
    first_used = global_id_->next_used(first_used);
    
    return AddrIterator(*this, first_used);
  }

  AddrIterator
  BDBImpl::end() const
  {
    return AddrIterator(*this, global_id_->end());  
  }

  void
  BDBImpl::stat(Stat *s) const
  {
    if(!s) return;
    bdbStater bstat(s);
    bstat(this);
  }
  
  bool
  BDBImpl::full() const
  { return !global_id_->avail(); }

  
  AddrType
  BDBImpl::write_pool(char const*data, uint32_t size)
  {
      unsigned int dir = addrEval.directory(size);
      if((unsigned int)-1 == dir)
        throw std::length_error(SRC_POS);

      AddrType rt(0), loc_addr(0);
      while(dir < addrEval.dir_count()){
        try{
          loc_addr = pools_[dir].write(data, size);
        }catch(addr_overflow const &){
          dir++;
          continue;
        }
        break;
      }

      if(dir >= addrEval.dir_count())
        throw addr_overflow();

      rt = addrEval.global_addr(dir, loc_addr);
      return rt;
  }

  void
  BDBImpl::error(int errcode, int line)
  {
    if(0 == err_log_) return;
    
    //lock
    
    if(0 == ftello(err_log_)){ // write column names
      fprintf(err_log_, "Pool ID\tLine\tMessage\n");
    }
    
    if(acc_log_) fprintf(acc_log_, "None    \t%d\t%s\n", line, error_num_to_str()(errcode));
    
    //unlock
  }

  void
  BDBImpl::error(unsigned int dir)
  { 
    // assert(0 != *this && "BDBImpl is not proper initiated");

    if(0 == err_log_) return;

    std::pair<int, int> err = pools_[dir].get_error();
    
    if(err.first == 0) return;

    // TODO lock log
    
    if(0 == ftello(err_log_)){ // write column names
      fprintf(err_log_, "Pool_ID  Line Message\n");
    }

    while(1){
      fprintf(err_log_, "%08x %4d %s\n", dir, err.second, error_num_to_str()(err.first));
      err = pools_[dir].get_error();
      if(err.first == 0) break;
    }

    // TODO unlock
  }

} // end of namespace BDB
