#ifndef BDB_ERROR_HPP
#define BDB_ERROR_HPP

#ifndef NDBUG
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
//#define LINE_POS STRINGIZE(__LINE__)
//#define FILE_POS STRINGIZE(__FILE__)
#define SRC_POS STRINGIZE(__FILE__) ":" STRINGIZE(__LINE__)
#else
#define SRC_POS
#endif

namespace BDB  {

  //! Error numbers
  enum ERRORNUMBER
  {
    /// Pool can not address more chunks
    ADDRESS_OVERFLOW = 1,

    /// I/O operation failure
    SYSTEM_ERROR = 2,

    /// Data is too big to be handled by BehaviorDB 
    DATA_TOO_BIG = 3,

    /// Pool is locked
    POOL_LOCKED = 4,

    NON_EXIST = 5,

    ROLLBACK_FAILURE = 6,

    COMMIT_FAILURE = 7
  };

  struct error_num_to_str
  {
    char const *operator()(int error_num)
    {
      return buf[error_num];
    }
  private:
    static char buf[8][40];
  };

  struct error_code
  {
    error_code();
  };

  struct addr_overflow{};

} // end of namespace BDB

#endif // end of header
