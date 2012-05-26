#ifndef _ADDR_EVAL_HPP
#define _ADDR_EVAL_HPP

#include "common.hpp"

#define BDB_MAXIMUM_CHUNK_SIZE_ (1<<31-1)

namespace BDB {
  
template<typename addr_t = AddrType>
struct addr_eval
{
  void
  init(   
    unsigned int dir_prefix_len, size_t min_size, 
    Chunk_size_est cse = &BDB::default_chunk_size_est, 
    Capacity_test ct = &BDB::default_capacity_test );

  bool
  is_init() const;
    
  void
  set(unsigned char dir_prefix_len);
  
  void
  set(size_t min_size);

  void
  set(Chunk_size_est chunk_size_estimation_func);

  void
  set(Capacity_test capacity_test_func);

  unsigned int
  global_addr_len() const;

  unsigned char
  local_addr_len() const;

  size_t 
  chunk_size_estimation(unsigned int dir) const;
  
  bool
  capacity_test(unsigned int dir, size_t size) const;

  unsigned int 
  dir_count() const;
  
  // estimate directory ID according to chunk size
  unsigned int 
  directory(size_t size) const;
  
  unsigned int 
  addr_to_dir(addr_t addr) const;

  addr_t 
  global_addr(unsigned int dir, addr_t local_addr) const;

  addr_t 
  local_addr(addr_t global_addr) const;
  
private:
  unsigned char dir_prefix_len_;
  size_t min_size_;
  
  Chunk_size_est chunk_size_est_;
  Capacity_test capacity_test_;

  addr_t loc_addr_mask;

}; // struct addr_eval

} // end of namespace BDB

#include "addr_eval.tcc"

#endif //end of header 
