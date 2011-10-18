#define _BDB_TESTING_
#include "array.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char** argv)
{
  using namespace std;
  using namespace BDB;

  Config conf;
  conf.beg = 1;
  conf.end = 10;
  conf.root_dir = argv[1];

  BehaviorDB bdb(conf);

  Array arr(100, "my_arr", bdb);
  cerr<<"is_acquired: "<<arr.is_acquired(1)<<endl;
  
  char buf[100];
  AddrType off = arr.put("aceryang", 8);
  assert(off != -1);

  arr.get(buf, 100, off);
  assert(0 == strncmp(buf, "aceryang", 8));
  
  off = arr.put("specified", 9, 1u);
  assert(off != -1);

  arr.get(buf, 100, 1u);
  assert(0 == strncmp(buf, "specified", 9));
}
