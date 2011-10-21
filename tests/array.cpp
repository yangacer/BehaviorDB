#define _BDB_TESTING_
#include "array.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char** argv)
{
  using namespace std;
  using namespace BDB;
  using BDB::Structure::Array;

  Config conf;
  conf.beg = 1;
  conf.end = 10;
  conf.root_dir = argv[1];

  BehaviorDB bdb(conf);
  char buf[100];

  {
    Array arr(100, "my_arr", bdb);
    cerr<<"is_acquired: "<<arr.is_acquired(1)<<endl;
  
    AddrType off = arr.put("aceryang", 8);
    assert(off != -1);

    arr.get(buf, 100, off);
    assert(0 == strncmp(buf, "aceryang", 8));
    
    assert(true == arr.del(off));

    off = arr.put("specified", 9, 1u);
    assert(off != -1);

    size_t rt = arr.get(buf, 100, 1u);
    assert(rt != -1 && 
      0 == strncmp(buf, "specified", 9));
    

  }
  
  
  {
    Array arr(200, "my_arr2", bdb);

    AddrType off = arr.put("tech", 4);
    assert(off != -1);
    
    arr.update("technology", 10, off);

    arr.get(buf, 100, off);
    assert(0 == strncmp(buf, "technology", 10));
  }
  
}
