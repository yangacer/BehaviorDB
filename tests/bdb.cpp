#include "bdb.hpp"
#include "addr_iter.hpp"
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <cmath>

void print_in_proper_unit(unsigned long long size)
{ 
  char const * units = " KMGT";
  unsigned long long n_size = size;
  unsigned int i=0;
  while(n_size > 1024){
    n_size>>=10;  
    ++i;
  }
  printf("%llu", n_size);
  printf(" %cB", units[i]);
  printf("(%llu)", size);
}

void usage()
{
  printf("./bdb work_dir/\n");
  exit(1);
}

int main(int argc, char** argv)
{
  using namespace BDB;

  if(argc < 2) usage();
  
  assert(BDB::npos != 0);

  printf("==== BehaviorDB Testing ====\n");
  printf("version: %s\n", BDB::VERSION);

  Config conf;
  conf.root_dir = argv[1];
  conf.min_size = 32;
  conf.beg = 0;

  { // ctor dtor testing
    printf(" - testing construction/deconstruction\n");
    {
      BehaviorDB bdb(conf);
      bdb.put("good", 4, 0u);
    }
    {
      BehaviorDB bdb(conf);
      char buf[4];
      bdb.get(buf, 4, 0u);
      assert(0 == strncmp(buf, "good", 4));
      bdb.del(0u);
    }
  }

  BehaviorDB bdb(conf);
  std::string should;
  std::string rec;
  char const* data = "acer";
  AddrType addrs[3] = {};

  {
    // write
    addrs[0] = bdb.put(data, 4);
    addrs[1] = bdb.put(data, 4);
    printf(" - write 4 bytes to two chunks\n");
    assert(addrs[0] == 1);
    assert(addrs[1] == 2);
    // write to specific chunk
    addrs[2] = bdb.put("good", 4, 3u);
    printf(" - write to address 3 which does not exist currently\n");
    assert(addrs[2] == 3);
  }

  {
    // read
    char read[5] = {};
    char read2[5] = {};

    bdb.get(read, 4, addrs[0]); 
    bdb.get(read2,4, addrs[1]);
    printf(" - read 4 bytes from two chunks\n");
    assert(0 == strncmp(data, read, 4));
    assert(0 == strncmp(data, read2, 4));
  }

  {
    // append include migration
    char const *data2 = "1234567890asdfghjkl;12345678901234567890";
    addrs[0] = bdb.put(data2, strlen(data2), addrs[0]);
    bdb.get(&rec, 50, addrs[0]);
    printf(" - append 40 bytes\n");
    assert(rec == "acer1234567890asdfghjkl;12345678901234567890");
  }
  
  {
    // prepend
    char const *data3 = "yang";

    should = data3;
    should += rec;
    addrs[0] = bdb.put(data3, 4, addrs[0], 0);
    bdb.get(&rec, 100, addrs[0]);
    printf(" - prepend then read\n");
    assert(should == rec);
  }
  
  {
    // insert
    char const *data4 = " made";
    
    addrs[0] = bdb.put(data4, 5, addrs[0], 8);
    should.insert(8, data4);
    bdb.get(&rec, 100, addrs[0]);

    printf(" - insert then read\n");
    assert(should == rec);
  }

  {
    // read into string
    bdb.get(&rec, 1024, addrs[0]);
    printf(" - read data into a std::string\n");
    assert(should == rec);
  }

  {
    // erase partial
    size_t nsize = bdb.del(addrs[0], 13, 10);
    bdb.get(&rec, 1024, addrs[0]);
    should.erase(13, 10);
    printf(" - del data betwen (13, 23]\n");
    assert(nsize == rec.size());
    assert(should == rec);
  }

  {
    // update
    bdb.update("replaced", addrs[0]);
    bdb.get(&rec, 1024, addrs[0]);
    should = "replaced";
    printf(" - replace data with \"replaced\"\n");
    assert(should == rec);
  }
  
  {
    // update to cause migration
    should = "123456789012345678901234567890tail";
    bdb.update(should.c_str(), should.size(), addrs[1]);
    bdb.get(&rec, 1024, addrs[1]);
    printf(" - replace data with long value to cause migration\n");
    assert(should == rec);
  }
  
  {
    AddrIterator iter = bdb.begin();
    printf(" - iterating all data\n");
    int i(0);
    while(iter != bdb.end()){
      assert(addrs[i] == *iter);
      ++iter; 
      ++i;
    }

    // interleave accesing will invalid an iterator
    // an out_of_range exception will be through
    iter = bdb.begin();
    bdb.del(addrs[0]);
    try{
      *iter;
      assert(false);
    }catch(std::exception const &e){}
  }

  // erase all again
  bdb.del(addrs[1]);
  bdb.del(addrs[2]);

  Stat stat;
  bdb.stat(&stat);
  printf("gid memory usage: ");
  print_in_proper_unit(stat.gid_mem_size);
  printf("\n");

  printf("pool memory usage: ");
  print_in_proper_unit(stat.pool_mem_size);
  printf("\n");

  printf("disk size: ");
  print_in_proper_unit(stat.disk_size);
  printf("\n");

  /**
   * XXX !!!Following code does not work currently.
   * (Hope I can get them back soon)
   *
  // streaming write
  rec = "toma";
  stream_state const* os = bdb.ostream(4);
  for(int i=0; i<4; ++i){
    os = bdb.stream_write(os, rec.data()+i, 1);
  }
  addr = bdb.stream_finish(os);
  rec.clear();
  bdb.get(&rec, 10, addr);
  printf("======== stream write (prototype)\n");
  printf("should: %08x\n", 4u);
  printf("result: %08x\n", addr);
  printf("should: %s\n", "toma");
  printf("result: %s\n", rec.c_str());
  bdb.del(addr);

  // streaming write to non-allocated addr
  os = bdb.ostream(4, 5);
  for(int i=0; i<4; ++i){
    os = bdb.stream_write(os, rec.data()+i, 1);
  }
  addr = bdb.stream_finish(os);
  rec.clear();
  bdb.get(&rec, 10, addr);
  printf("======== stream write to non-allocated addr\n");
  printf("should: %08x\n", 5u);
  printf("result: %08x\n", addr);
  printf("should: %s\n", "toma");
  printf("result: %s\n", rec.c_str());
  bdb.del(addr);
  */
  return 0; 
}
