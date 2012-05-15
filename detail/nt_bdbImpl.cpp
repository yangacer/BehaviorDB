#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "idPool.hpp"
#include "error.hpp"

namespace BDB {
  
  AddrType
  BDBImpl::nt_put(char const *data, size_t size)
  {
    
    AddrType rt = write_pool(data, size);
    if(-1 == rt) return -1;

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\n", "nt_put", size);

    return rt;
  }
  
  AddrType
  BDBImpl::nt_put(char const* data, size_t size, AddrType addr, size_t off)
  {
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    AddrType rt;

    ChunkHeader header;
    if(-1 == pools_[dir].head(&header, loc_addr)){
      error(dir); 
      return -1;
    }

    if( size + header.size > addrEval.chunk_size_estimation(dir)){

      // migration
      unsigned int next_dir = 
        addrEval.directory(size + header.size);
      if((unsigned int)-1 == next_dir){
        error(DATA_TOO_BIG, __LINE__);
        return -1;
      }
      AddrType next_loc_addr;
      if(npos == off)
        off = header.size;

      // TODO migrate failure 
      next_loc_addr = pools_[dir].merge_move( 
          data, size, loc_addr, off,
          &pools_[next_dir], &header); 

      if(-1 == next_loc_addr){
        error(dir);
        error(next_dir);
        return -1;  
      }
      rt = addrEval.global_addr(next_dir, next_loc_addr);
      if(acc_log_)fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
        "nt_insert", size, rt, off);
      return rt;
    }

    // no migration
    // **Althought the chunk need not migrate to another pool, 
    // it might be moved to another chunk of the same pool 
    // due to size of data to be moved exceed size of 
    // migration buffer that a pool contains
    if(-1 == (loc_addr = 
      pools_[dir].write(data, size, loc_addr, off, &header)) )
    {
      error(dir);
      return -1;  
    }
    
    rt = addrEval.global_addr(dir, loc_addr);

    if(acc_log_)fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
      "nt_insert", size, rt, off);

    return rt;
  }
  
  /// TODO this method should called "replace"
  AddrType
  BDBImpl::nt_update(char const *data, size_t size, AddrType addr)
  {

    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    // check size
    if( !addrEval.capacity_test(dir, size) ){
      
      unsigned int old_dir = dir;
      
      AddrType old_loc_addr = loc_addr;

      AddrType new_addr =
        write_pool(data, size);

      if(-1 == new_addr)
        return -1;  
      
      if(-1 == pools_[old_dir].free(old_loc_addr)){
        error(old_dir);
        return -1;
      }

      if(acc_log_) 
        fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "nt_upd_put", size, new_addr);
      return new_addr;
    }
    
    if(-1 == (loc_addr = 
      pools_[dir].replace(data, size, loc_addr)) )
    {
      error(dir);
      return -1;  
    }

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\n", "nt_update", size, addr);

    return addr;
  }
  
  size_t
  BDBImpl::nt_get(char *output, size_t size, AddrType addr, size_t off)
  {
    size_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    if(-1 == (rt = pools_[dir].read(output, size, loc_addr, off))){
      error(dir);
      return -1;
    }

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "nt_get", size, addr, off);

    return rt;
  }
  
  size_t
  BDBImpl::nt_get(std::string *output, size_t max, AddrType addr, size_t off)
  {
    assert(0 != output && "Non-allocate output");

    size_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    if( -1 == (rt = pools_[dir].read(output, max, loc_addr, off))){
      error(dir);
      return -1;
    }
    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "nt_strget", output->size(), addr, off);
    return rt;
  }
  
  size_t
  BDBImpl::nt_del(AddrType addr)
  {
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    if(-1 == pools_[dir].free(loc_addr)){
      error(dir);
      return -1;  
    }

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\n", "nt_del", addr);
    return 0;
  }

  size_t
  BDBImpl::nt_del(AddrType addr, size_t off, size_t size)
  {
  
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    size_t nsize;
    if(-1 == (nsize = pools_[dir].erase(loc_addr, off, size))){
      error(dir);
      return -1;
    }
    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
        "nt_part_del", addr, off, size);
    return nsize;
  }

} // namespace BDB
