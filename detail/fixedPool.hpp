#ifndef FIXEDPOOL_HPP_
#define FIXEDPOOL_HPP_

#include "common.hpp"
#include <string>
#include <cstdio>
#include <vector>

namespace BDB {

  /** @brief Fixed size data pool
   *  @tparam T data type
   *  @tparam TextSize Size of "serializaed" data
   */
  template<typename T, uint32_t TextSize>
  struct fixed_pool
  {
    typedef T value_type;
    typedef T& reference;
    
    fixed_pool(uint32_t /*dummy*/); 
    
    // TODO deprecate this
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
    void open(unsigned int id, char const* work_dir);
    
    T operator[](AddrType addr) const;
    
    void init(T const &val, AddrType off){}
    
    void store(T const &val, AddrType off);
    
    void resize(uint32_t size){};

    int read(T* val, AddrType addr) const;
    
    int write(T const & val, AddrType addr);

    std::string dir() const;

  private:
    unsigned int id_;
    std::string work_dir_;
    FILE* file_;
    char *fbuf_;//[4096];
  };
  
  
  template<typename T>
  struct vec_wrapper
  {
    typedef T value_type;
    typedef T& reference;
    typedef typename std::vector<T>::size_type size_type;
      
    vec_wrapper(uint32_t size)
    : vec_(size)
    {}

    void open(unsigned int id, char const* work_dir)
    {}

    T operator[](AddrType addr) const
    { return vec_[addr]; }

    void init(T const &val, AddrType off)
    { vec_[off] = val; }

    void store(T const &val, AddrType off)
    { vec_[off] = val; }
    
    void resize(size_type size)
    { vec_.resize(size); }

  private:
    std::vector<T> vec_;
  };
  
}

#endif // end of header
