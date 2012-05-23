#ifndef BDB_EXCEPTION_HPP_
#define BDB_EXCEPTION_HPP_

#include "export.hpp"
#include "common.hpp"

namespace BDB {

struct BDB_EXPORT addr_overflow{};
struct BDB_EXPORT chunk_overflow{};
struct BDB_EXPORT invalid_addr{};

}// namespace BDB
#endif
