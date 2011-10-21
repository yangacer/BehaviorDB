#include "hash_map.hpp"
#include <iostream>

int main()
{
  using namespace std;
  using namespace BDB;
  using BDB::Structure::HashMap;
  
  Config conf;
  conf.beg = 1;
  conf.end = 10;
  conf.root_dir = argv[1];

  BehaviorDB bdb(conf);

  HashMap<int> hmap("my_map", bdb);

  string buf;  

  hmap.put("key", "value");

  hmap.is_in("key");

  hmap.get("key", &buf);

  hmap.get_bucket("key");

  size_t hv = hmap.hash_function("key");
  
  HashMap<int>::BucketType bucket;

  hmap.get_bucket(hv, &bucket);

  for(HashMap<int>::BucketType::iterator it = bucket.begin();
    it != bucket.end(); ++it)
    cout<<it->first<<":"<<it->second<<"\n";

  bucket.erase(bucket.begin());

  hmap.update_bucket("key", bucket);

  
  return 0;
}
