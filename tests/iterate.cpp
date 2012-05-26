#include "bdb.hpp"
#include "addr_iter.hpp"
#include <cstdio>

int main(int argc, char** argv)
{
  using namespace std;
  using namespace BDB;

  Config conf;
  conf.root_dir = argv[1];
  
  BehaviorDB bdb(conf);

  AddrIterator iter = bdb.begin();
  while(iter != bdb.end()){
    printf("%08x\n", *iter);
    ++iter;
  }
  return 0;
}
