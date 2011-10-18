#include <vector>
#include <iostream>
// you have to include serialization of vector
#include "boost/serialization/vector.hpp"
#include "typed_array.hpp"

int main(int argc, char **argv)
{
  using namespace std; 
  using namespace BDB;

  Config conf;
  conf.beg = 1;
  conf.end = 10;
  conf.root_dir = argv[1];

  BehaviorDB bdb(conf);

  TypedArray<vector<int> > tarr("my_tarr", bdb);

  vector<int> data(3), dest(3);
  data[0] = 1;
  data[1] = 4;
  data[2] = 123;

  AddrType off = tarr.put(data);
  assert(off != -1);

  assert(true == tarr.get(&dest, off));

  for(int i=0;i<dest.size();++i)
    cerr<<dest[i]<<",";
  cerr<<"\n";
}
