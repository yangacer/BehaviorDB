#ifndef BDB_EXCEPTION_HPP_
#define BDB_EXCEPTION_HPP_

#include "export.hpp"
#include "common.hpp"

namespace BDB {

struct BDB_API addr_overflow{};
struct BDB_API chunk_overflow{};
struct BDB_API invalid_addr{};
struct BDB_API data_currupted{AddrType addr;};

}// namespace BDB
#endif
