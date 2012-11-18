#ifndef BDB_LOG_HPP_
#define BDB_LOG_HPP_
#include <ostream>
#include <utility>

/* TODO Make Config can be serialized to log file
#include "common.hpp"

template<class Ar>
void serialize(Ar &ar, Config &conf, unsigned int const version)
{
  ar & conf.beg & conf.end & addr_prefix_len;
  ar & min_size & root_dir & pool_dir;
  ar & trans_dir & header_dir & log_dir;
}
*/
namespace BDB {

inline void log_(std::ostream &os)
{ os << "\n"; }

template <typename First, typename ...Rest>
void log_(std::ostream &os, First&& first, Rest&&... rest)
{  
  os << " " << first; 
  log_(os, std::forward<Rest>(rest)...);
}

class logger 
{
public:
  logger(std::ostream &os) : os_(os){}
  ~logger() {}
  
  template<typename ...Args>
  void log(std::string const &signature, Args&&... args)
  { 
    os_ << signature;
    log_(os_, std::forward<Args>(args)...);
  }

private:
  std::ostream &os_;
};

} // namespace BDB

#endif // header guard
