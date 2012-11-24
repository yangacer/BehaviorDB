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

#define MIGBUF_SIZ (1<<20)

namespace BDB {

  struct viov;

  template<typename T>
  class IDPool;
  
  template<typename T>
  struct id_handle;

  /// pool manager
  //! \callgraph
  struct pool
  {
    friend struct bdbStater;
    
    /// pool configuration
    struct config
    {
      unsigned int dirID;
      std::string work_dir;
      std::string trans_dir;
      std::string header_dir;
      
      config() : dirID(0)  {}
    };

    pool(config const &conf, addr_eval<AddrType> &addrEval);
    ~pool();
    
    /** @brief Write new data
     *  @param data
     *  @param size
     *  @return Address
     */
    AddrType
    write(char const* data, uint32_t size);
    
    /** off = BDB::npos represents an append write
     */
    AddrType
    write(char const* data, uint32_t size, AddrType addr, uint32_t off=npos);
    
    AddrType
    write(viov *vv, uint32_t len);
    
    AddrType
    replace(char const *data, uint32_t size, AddrType addr);

    uint32_t
    read(char* buffer, uint32_t size, AddrType addr, uint32_t off=0);
    
    uint32_t
    read(std::string *buffer, uint32_t max, AddrType addr, uint32_t off=0);
    
    AddrType
    merge_copy(
      char const* data, 
      uint32_t size, 
      AddrType src_addr, 
      uint32_t off, 
      pool* dest_pool);

    AddrType
    merge_move(
      char const* data, 
      uint32_t size, 
      AddrType src_addr, 
      uint32_t off, 
      pool *dest_pool);
  
    AddrType
    merge_erase(
      uint32_t size,
      AddrType src_addr,
      uint32_t off,
      pool* dest_pool);

    uint32_t
    free(AddrType addr);

    // TODO deprecate this and use merge_erase
    uint32_t
    erase(AddrType addr, uint32_t off, uint32_t size);
  
    /** Overwrite chunk data
     *  @remark This method ONLY checks the written 
     *  data do not exceed chunk boundary. It does NOT change
     *  chunk header (for data size). For better data
     *  integrity, this method should be used
     *  with merge_copy(...).
     *  i.e.
     *  @code
     *  // copy data refered by the exist_addr to a new address
     *  // one can control position of the room by 'off' param
     *  // here we use 'data_end' to perform an 'append' operation
     *  // caller should determine which pool to place the copy
     *  uint32_t data_end = end_position_of_the_original_data;
     *  AddrType addr = merge_copy(0, 128, exist_addr, data_end, dest_pool);
     *  if(128 != overwrite(new_data, 128, addr, data_end))
     *    free(addr); // abort, no change to the original chunk
     *  
     *  // success
     *  @endcode
     *  @throw invalid_addr
     */
    uint32_t
    overwrite(char const* data, uint32_t size, AddrType addr, uint32_t off);

    // --------- misc -----------
    void
    pine(AddrType addr);

    void
    unpine(AddrType addr);
    
    bool
    is_pinned(AddrType addr);
    
  private:

    off_t
    seek(AddrType addr, uint32_t off =0);

    off_t
    addr_off2tell(AddrType addr, uint32_t off) const;
    
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
    char *file_buf_;
    
    typedef IDPool<fixed_pool<ChunkHeader,8> > idpool_t;
    typedef id_handle<idpool_t> id_handle_t;

    idpool_t *idpool_;
  };
} // end of namespace BDB


#endif // end of pool.hpp guard
