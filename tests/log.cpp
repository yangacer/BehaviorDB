#include "log.hpp"
#include <sstream>
#include <iostream>

int main()
{
  std::stringstream sin;
  BDB::logger l(sin);

  l.log("put", 1024, 1238, "char const*");

  std::cout << sin.str() << std::endl;

  return 0;
}
