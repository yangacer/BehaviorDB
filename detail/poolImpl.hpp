#ifndef _POOL_HPP_
#define _POOL_HPP_

#include "common.hpp"
#include "addr_eval.hpp"
#include "fixedPool.hpp"
#include "chunk.h"
#include <string>
#include <cstdlib>
#include <deque>
#include <utility>

#define MIGBUF_SIZ 2*1024*1024


namespace BDB
{
  
  struct viov;
  class IDPool;

  /// pool manager
  //! \callgraph
  struct pool
  {
    friend struct bdbStater;
    
    /// pool configuration
    struct config
    {
      unsigned int dirID;
      char const* work_dir;
      char const* trans_dir;
      char const* header_dir;
      //addr_eval<AddrType> * addrEval;
      
      config()
      : dirID(0), 
        work_dir(""), trans_dir(""), header_dir("")//,
        //addrEval(0)
      {}
    };

    pool(config const &conf, addr_eval<AddrType> &addrEval);
    ~pool();
    
    operator void const*() const;
    
    /** @brief Write new data
     *  @param data
     *  @param size
     *  @return Address
     */
    AddrType
    write(char const* data, size_t size);
    
    // off=-1 represent an append write
    AddrType
    write(char const* data, size_t size, AddrType addr, size_t off=npos, ChunkHeader const* header=0);
    
    AddrType
    write(viov *vv, size_t len);
    
    AddrType
    replace(char const *data, size_t size, AddrType addr, ChunkHeader const *header=0);

    size_t
    read(char* buffer, size_t size, AddrType addr, size_t off=0, ChunkHeader const* header=0);
    
    size_t
    read(std::string *buffer, size_t max, AddrType addr, size_t off=0, ChunkHeader const* header=0);
    
    AddrType
    merge_copy(char const* data, size_t size, AddrType src_addr, 
      size_t off, pool* dest_pool, ChunkHeader const* header=0);

    AddrType
    merge_move(char const* data, size_t size, AddrType src_addr, 
      size_t off, pool *dest_pool, ChunkHeader const* header=0);

    size_t
    free(AddrType addr);

    size_t
    erase(AddrType addr, size_t off, size_t size);
  
    /** Overwrite chunk data
     *  @remark This method ONLY checks the written 
     *  data be in a chunk area. It does NOT change
     *  chunk header (for data size). For better data
     *  integrate, this method should be used
     *  with merge_copy(...).
     *  i.e.
     *  @code
     *  // copy data refered by the exist_addr to a new address
     *  // one can control position of the room by 'off' param
     *  // here we use 'data_end' to perform an 'append' operation
     *  // caller should determine which pool to place the copy
     *  size_t data_end = end_position_of_the_original_data;
     *  AddrType addr = merge_copy(0, 128, exist_addr, data_end, dest_pool);
     *  if(128 != overwrite(new_data, 128, addr, data_end)){
     *  free(addr); // abort, no change to the original chunk
     *  }
     *  // success
     *  @endcode
     */
    size_t
    overwrite(char const* data, size_t size, AddrType addr, size_t off);

    // misc 

    int
    head(ChunkHeader *header, AddrType addr) const;

    void
    on_error(int errcode, int line);
    
    std::pair<int, int>
    get_error();

    void
    pine(AddrType addr);

    void
    unpine(AddrType addr);
    
    bool
    is_pinned(AddrType addr);

    /* TODO: To be considered
    std::pair<AddrType, size_t>
    tell2addr_off(off_t fpos) const;
    */
    
  private:
    off_t
    seek(AddrType addr, size_t off =0);

    off_t
    addr_off2tell(AddrType addr, size_t off) const;
    
    /*
    void lock_acq();
    void lock_rel();
    */

    pool(pool const& cp);
    pool& operator=(pool const& cp);

    // data membera
    addr_eval<AddrType> const & addrEval;
    unsigned int dirID;
    std::string work_dir;
    std::string trans_dir;
    
    // pool file
    FILE *file_;
    char *mig_buf_;
    char *file_buf_;
    // id file
    IDPool *idPool_;

    // header
    fixed_pool<ChunkHeader, 8> headerPool_;
  public: 
    std::deque<std::pair<int,int> > err_;
  };
} // end of namespace BDB


#endif // end of pool.hpp guard
