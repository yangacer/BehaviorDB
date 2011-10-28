#include "hash_map.hpp"
#include <iostream>

int main()
{
  using namespace std;
  using namespace BDB;
  using namespace BDB::Structure;
  
  Config conf;
  conf.beg = 1;
  conf.end = 10;
  conf.root_dir = argv[1];

  BehaviorDB bdb(conf);
  
  Array arr("my_arr", bdb);

  HashMap hmap("my_hmap", arr, bdb);

  string buf;  

  hmap.put("key", "value");

  hmap.is_in("key");

  hmap.get("key", &buf);

  hmap.del("key");

  return 0;
}
