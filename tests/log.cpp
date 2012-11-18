#include "log.hpp"
#include <sstream>
#include <cassert>

int main()
{
  std::stringstream sin;
  BDB::logger l(sin);

  l.log("put", 1024, 1238, "char-const*");

  assert(sin.str() == "put 1024 1238 char-const*\n");

  return 0;
}
