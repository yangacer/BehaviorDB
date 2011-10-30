#include "hash_map.hpp"
#include <iostream>

int main(int argc, char** argv)
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

  HashMap hmap(20, "my_hmap", arr, bdb);

  string buf;  

  assert(-1 != hmap.put("key", "value") && "put by hashmap failed");

  assert(false != hmap.is_in("key") && "is_in test failed");

  hmap.get("key", &buf);
  assert(buf == "value" && "get by hashmap failed");

  assert(true == hmap.del("key") && "del by hashmap failed");
  
  
  //assert(false == hmap.is_in("non-existed"));
  
  assert(-1 == hmap.get("key", &buf) && "del is not correct");
  
  

  return 0;
}
