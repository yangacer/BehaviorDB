#ifndef _COMMON_HPP
#define _COMMON_HPP

#include "export.hpp"
#include <string>

#ifdef __GNUC__ // GNU

#include <stdint.h>

#elif defined(_WIN32) || defined(_WIN64) // MSVC

#pragma warning( disable: 4290 ) // exception specification non-implmented
#pragma warning( disable: 4251 ) // template export warning
#pragma warning( disable: 4996 ) // allow POSIX 

typedef unsigned __int32 uint32_t;

#endif

namespace BDB {
  
  typedef uint32_t AddrType;
  struct stream_state;
  /// Prototype of chunk size estimation callback.
  typedef uint32_t (*Chunk_size_est)(unsigned int dir, uint32_t min_size);
  /// Prototype of chunk cacpcity testing callback.
  typedef bool (*Capacity_test)(uint32_t chunk_size, uint32_t data_size);
  /**@brief Default chunk size estimation callback.
  */
  inline uint32_t 
  default_chunk_size_est(unsigned int dir, uint32_t min_size)
  {
    return min_size<<dir;
  }

  /**@brief Default capacity testing callback
   * @details Decide how many fragmentation is acceptiable for initial 
   * data insertion. 
   * Or, w.r.t. preallocation, how many fraction of a chunk one want to preserve
   * for future insertion.
   * It should return true if data/chunk_size is acceptiable, otherwise it return false.
   */
  inline bool 
  default_capacity_test(uint32_t chunk_size, uint32_t data_size)
  {
    return (chunk_size - (chunk_size>>2)) >= data_size;
  }

  /** @brief Configuration of BehaviorDB */
  struct BDB_API Config
  {
    /// Begin number of global IDs of BehaviorDB
    AddrType beg;
    /// End number of global IDs of BehaviorDB (the one after the last ID)
    AddrType end;
    /// Bit length of address prefixes. 
    /** This parameter limits how many pools a BehaviorDB can have.
    */
    unsigned int addr_prefix_len;
    /// Minimum chunk size of BehaviorDB 
    uint32_t min_size;
    //! Directory for placing files that are used by BehaviorDB. 
    /** Default is an empty string so that a BehaviorDB uses current
     *  directory as a root dir.
     */
    std::string root_dir;
    /// Directory for placing pool files. Default is the root_dir
    std::string pool_dir;
    /// Directory for placing transaction files. Default is the root_dir.
    std::string trans_dir;
    /// Directory for placing header files. Default is the root_dir.
    std::string header_dir;
    /// Directory for placing log file. Default is the root_dir.
    std::string log_dir;
    /// Chunk size estimation callback
    Chunk_size_est cse_func;
    /// Capacity testing callback
    Capacity_test ct_func;
    /** @brief Config default constructor 
     *  @details Construct BDB::Config with default configurations  
     */
    Config(
      AddrType beg = 1,
      AddrType end = 100000001,
      unsigned int addr_prefix_len = 4,
      uint32_t min_size = 32,
      char const *root_dir = "",
      char const *pool_dir = "",
      char const *trans_dir = "",
      char const *header_dir = "",
      char const *log_dir = "",
      Chunk_size_est cse_func = &default_chunk_size_est,
      Capacity_test ct_func = &default_capacity_test 
    );

    /** @brief Validate configuration
     *  @throw invalid_argument
     */
    void validate() const;
  };

  /// Memory/Disk Statistic TODO fragmentation
  struct BDB_API Stat
  {
    /// global ID table byte size
    unsigned long long gid_mem_size;
    /// pool byte size
    unsigned long long pool_mem_size;
    /// disk usage
    unsigned long long disk_size;
    Stat()
    :gid_mem_size(0), pool_mem_size(0), disk_size(0)
    {}
  };

  /// Not a Position
  extern const uint32_t npos;
  /// Version information
  extern char const* VERSION;
} // end of nemaespace BDB

#endif // end of header
