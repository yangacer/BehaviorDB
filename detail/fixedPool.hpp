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

    fixed_pool() 
      : id_(0), work_dir_(""), file_(0), fbuf_(0)
    {}

    fixed_pool(unsigned int id, char const* work_dir)
      : id_(0), work_dir_(""), file_(0), fbuf_(0)
    {
      open(id, work_dir);
    }

    ~fixed_pool()
    {
      if(file_) fclose(file_);
      delete []fbuf_;
    }

    operator void const *() const
    { 
      if(!file_) return 0;
      return this;
    }

    /** Open pool file
     * @param id 		 
     * @param work_dir
     * @throw length_error For overflowed pathname
     * @throw invalid_argument For invalid pathname
     * @remark If id is 1, then this object will be 
     * associated with a file "0001.fpo".
     */
    void
      open(unsigned int id, char const* work_dir)
      {
        using namespace std;

        id_ = id;
        work_dir_ = work_dir;

        char fname[256];
        if(work_dir_.size() > 256) 
          throw length_error("fixed_pool: length of pool_dir string is too long");

        sprintf(fname, "%s%04x.fpo", work_dir_.c_str(), id_);

        fbuf_ = new char[4096];

        if(0 == (file_ = fopen(fname, "r+b"))){
          if(0 == (file_ = fopen(fname, "w+b"))){
            string msg("fixed_pool: Unable to create fix pool ");
            msg += fname;
            throw invalid_argument(msg.c_str());
          }
        }
        setvbuf(file_, fbuf_, _IOFBF, 4096);

      }

    int read(T* val, AddrType addr) const
    {
      if(!*this) return -1;

      off_t loc_addr = addr;
      loc_addr *= TextSize;
      if(-1 == fseeko(file_, loc_addr, SEEK_SET))
        return -1;
      if( 0 == file_>>*val)
        return -1;
      if(ferror(file_)) return -1;
      return 0;
    }

    int write(T const & val, AddrType addr)
    {
      if(!*this) return -1;

      off_t loc_addr = addr;
      loc_addr *= TextSize;
      if(-1 == fseeko(file_, loc_addr, SEEK_SET))
        return -1;
      if( 0 == file_<<val ) return -1;
      if(ferror(file_)) return -1;
      if(fflush(file_)) return -1;
      return 0;
    }

    std::string
      dir() const 
      {
        return work_dir_;	
      }

  private:
    unsigned int id_;
    std::string work_dir_;
    FILE* file_;
    char *fbuf_;//[4096];
  };
}

#endif // end of header
