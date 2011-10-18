#include "array.hpp"
#include <iostream>

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

  AddrType addr = arr.acquire();
  if(-1 != addr )
    arr.commit(addr);

  if(arr.acquire(1, 1234))
    arr.commit(1);
  else
    cerr<<"acquire address 1 failed\n";

  arr.release(addr);
  arr.commit(addr);

}
