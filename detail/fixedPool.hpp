#ifndef FIXEDPOOL_HPP_
#define FIXEDPOOL_HPP_

#include "common.hpp"
#include <string>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cassert>

namespace BDB {

  /** @brief Fixed size data pool
   *  @tparam T data type
   *  @tparam TextSize Size of "serializaed" data
   */
  template<typename T, size_t TextSize>
  struct fixed_pool
  {
    typedef T value_type;

    fixed_pool(); 

    fixed_pool(unsigned int id, char const* work_dir);

    ~fixed_pool();

    operator void const *() const;

    /** Open pool file
     * @param id  
     * @param work_dir
     * @throw length_error For overflowed pathname
     * @throw invalid_argument For invalid pathname
     * @remark If id is 1, then this object will be 
     * associated with a file "0001.fpo".
     */
    void
    open(unsigned int id, char const* work_dir);

    int read(T* val, AddrType addr) const;
    
    int write(T const & val, AddrType addr);

    std::string dir() const;

  private:
    unsigned int id_;
    std::string work_dir_;
    FILE* file_;
    char *fbuf_;//[4096];
  };
}

#endif // end of header
