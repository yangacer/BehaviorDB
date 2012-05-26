#include "bdb.hpp"
#include "addr_iter.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <cmath>

void print_in_proper_unit(unsigned long long size)
{ 
  char const * units = " KMGT";
  unsigned int i=0;
  while(size > 1024){
    size>>=10;  
    ++i;
  }
  printf("%llu", size);
  printf(" %cB", units[i]);
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

  printf("==== BehaviorDB Testing ====\n");
  printf("version: %s\n", BDB::VERSION);

  Config conf;
  conf.root_dir = argv[1];
  
  // Uncomment following two lines causes bad_alloc()
  //conf.min_size = 10 * 1024 * 1024;
  //conf.addr_prefix_len = 10;
  BehaviorDB bdb(conf);
  std::string should;
  size_t off;

  // write
  char const* data = "acer";
  AddrType addr = bdb.put(data, 4);
  AddrType addr2 = bdb.put(data, 4);
  printf("\n==== write 4 bytes to two chunks ====\n");
  printf("write \"%s\"\n", data);
  printf("should: 00000001\n");
  printf("result: %08x\n", addr); 
  printf("write \"%s\"\n", data);
  printf("should: 00000002\n");
  printf("result: %08x\n", addr2);  

  // write to specific chunk
  AddrType addr3 = bdb.put("good", 4, 3u);
  printf("\n=== write to address 3 which does not exist currently ====\n");
  printf("should: 00000003\n");
  printf("result: %08x\n", addr3);

  // read
  char read[5] = {};
  char read2[5] = {};

  bdb.get(read, 4, addr); 
  bdb.get(read2,4, addr2);
  printf("\n==== read 4 bytes from two chunks ====\n");
  printf("should: %s\n", data);
  printf("result: %s\n", read);
  printf("should: %s\n", data);
  printf("result: %s\n", read2);

  // append include migration
  char const *data2 = "1234567890asdfghjkl;12345678901234567890";
  std::string rec;
  addr = bdb.put(data2, strlen(data2), addr);
  bdb.get(&rec, 50, addr);
  printf("\n==== append 40 bytes ====\n");
  printf("append \"%s\" after \"%s\"\n", data2, data);
  printf("should: acer1234567890asdfghjkl;12345678901234567890\n");
  printf("result: %s\n", rec.c_str()); 


  // prepend
  char const *data3 = "yang";
  should = data3;
  should += rec;

  addr = bdb.put(data3, 4, addr, 0);
  rec.clear();
  bdb.get(&rec, 100, addr);

  printf("\n==== insertion then read====\n");
  printf("prepend \"yang\" and read result into a c-string\n");
  printf("should:\t%s\n", should.c_str());
  printf("result:\t%s\n", rec.c_str());

  // insert
  char const *data4 = " made";
  addr = bdb.put(data4, 5, addr, 8);

  should.insert(8, data4);
  rec.clear();
  // read loop
  char buf[4];
  /*
  size_t off(0), readCnt(0);
  while(0 < (readCnt = bdb.get(buf, 4, addr, off))){
    rec.append(buf, readCnt);
    off += readCnt;
  }
  */
  bdb.get(&rec, 100, addr);
  printf("\n==== insertion then read====\n");
  printf("insert \" made\" at 8, and read result into a c-string\n");
  printf("should:\t%s\n", should.c_str());
  printf("result:\t%s\n", rec.c_str());

  // read into string
  rec.clear();
  bdb.get(&rec, 1024, addr);
  printf("\n==== read data into a std::string ====\n");
  printf("should:\t%s\n", should.c_str());
  printf("result:\t%s\n", rec.c_str());

  // erase partial
  size_t nsize = bdb.del(addr, 13, 10);
  bdb.get(&rec, 1024, addr);
  should.erase(13, 10);
  printf("\n==== del data betwen (13, 23] ====\n");
  printf("should:\t%s\t%d\n", should.c_str(), should.size());
  printf("result:\t%s\t%d\n", rec.c_str(), nsize);

  // update
  bdb.update("replaced", addr);
  bdb.get(&rec, 1024, addr);
  should = "replaced";
  printf("\n==== replace data with \"replaced\" ====\n");
  printf("should:\t%s\n", should.c_str());
  printf("result:\t%s\n", rec.c_str());

  // update to cause migration
  should = "123456789012345678901234567890tail";
  bdb.update(should.c_str(), should.size(), addr2);
  bdb.get(&rec, 1024, addr2);
  printf("\n==== replace data with long value to cause migration ====\n");
  printf("should:\t%s\n", should.c_str());
  printf("result:\t%s\n", rec.c_str());

  AddrType addrs[] = { 1, 2, 3 };

  AddrIterator iter = bdb.begin();
  int i=0;
  printf("\n==== iterating all data ==== \n");

  while(iter != bdb.end()){
    printf("should: %08x\n", addrs[i]);
    printf("result: %08x\n", *iter);
    ++iter; 
    ++i;
  }

  // interleave accesing will invalid an iterator
  // an out_of_range exception will be through
  iter = bdb.begin();
  bdb.del(addrs[0]);
  try{
    *iter;
  }catch(std::exception const &e){
    printf("exception: %s\n", e.what());  
  }

  // erase all again
  //bdb.del(addrs[1]);
  //bdb.del(addrs[2]);

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

  /*
  // streaming write
  rec = "toma";
  stream_state const* os = bdb.ostream(4);
  for(int i=0; i<4; ++i){
    os = bdb.stream_write(os, rec.data()+i, 1);
  }
  addr = bdb.stream_finish(os);
  rec.clear();
  bdb.get(&rec, 10, addr);
  printf("======== stream write (prototype) ========\n");
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
  printf("======== stream write to non-allocated addr ========\n");
  printf("should: %08x\n", 5u);
  printf("result: %08x\n", addr);
  printf("should: %s\n", "toma");
  printf("result: %s\n", rec.c_str());
  bdb.del(addr);
  */
  return 0; 
}
