#include "bdbImpl.hpp"
#include "poolImpl.hpp"
#include "idPool.hpp"
#include "error.hpp"

namespace BDB {
  
  AddrType
  BDBImpl::nt_put(char const *data, size_t size)
  {
    AddrType rt = write_pool(data, size);
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

    try{
      // no migration
      loc_addr = pools_[dir].write(data, size, loc_addr, off);
      rt = addrEval.global_addr(dir, loc_addr);
      if(acc_log_)
        fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
                "insert", size, rt, off);
    }catch(internal_chunk_overflow const &co){
      // migration
      unsigned int next_dir = 
        addrEval.directory(size + co.current_size);
      AddrType next_loc_addr;
      
      if(npos == next_dir)
        throw chunk_overflow();
      if(npos == off)
        off = co.current_size;

      while(next_dir < addrEval.dir_count()){
        try{
          next_loc_addr = 
            pools_[dir].
            merge_move(data, size, loc_addr, off,
                       &pools_[next_dir]);
        }catch(addr_overflow const &){
          next_dir++;
          continue;
        }
        break;
      }
      if( next_dir >= addrEval.dir_count())
        throw addr_overflow();

      rt = addrEval.global_addr(next_dir, next_loc_addr);
      if(acc_log_)fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
        "insert", size, rt, off);
    }
    return addr;
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
      AddrType old_addr = loc_addr;
      AddrType addr = write_pool(data, size);
      pools_[old_dir].free(old_addr);
    }else{
      addr = pools_[dir].replace(data, size, loc_addr);
    }

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\n", 
              "update_put", size, addr);

    return addr;
  }
  
  size_t
  BDBImpl::nt_get(char *output, size_t size, AddrType addr, size_t off)
  {
    size_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    rt = pools_[dir].read(output, size, loc_addr, off);

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "nt_get", size, addr, off);

    return rt;
  }
  
  size_t
  BDBImpl::nt_get(std::string *output, size_t max, AddrType addr, size_t off)
  {
    if(!output) 
      throw std::invalid_argument(SRC_POS);

    size_t rt(0);
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    rt = pools_[dir].read(output, max, loc_addr, off);

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", "nt_strget", output->size(), addr, off);
    return rt;
  }
  
  size_t
  BDBImpl::nt_del(AddrType addr)
  {
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    
    pools_[dir].free(loc_addr);

    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\n", "nt_del", addr);
    return 0;
  }

  size_t
  BDBImpl::nt_del(AddrType addr, size_t off, size_t size)
  {
    unsigned int dir = addrEval.addr_to_dir(addr);
    AddrType loc_addr = addrEval.local_addr(addr);
    size_t nsize = pools_[dir].erase(loc_addr, off, size);
    
    if(acc_log_) 
      fprintf(acc_log_, "%-12s\t%08x\t%08x\t%08x\n", 
        "nt_part_del", addr, off, size);

    return nsize;
  }

} // namespace BDB
