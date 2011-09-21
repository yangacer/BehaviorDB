/** @file bdb.cpp
 *  @example bdb.cpp
 */



#include "bdb.hpp"
#include "addr_iter.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <exception>
#include <cassert>
#include <cstdlib>
#include <cmath>

/// print byte size in proper unit
void print_in_proper_unit(unsigned long long size)
{	
	char const * units = " KMGT";
	unsigned int i=0;
	while(size > 1024){
		size>>=10;	
		++i;
	}
	printf("%u", size);
	printf(" %cB", units[i]);
}

/// echo usage and exit
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

	conf.min_size = 32;
	BehaviorDB bdb(conf);
	

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
	
	bdb.del(addr2);

	// append include migration
	char const *data2 = "1234567890asdfghjkl;12345678901234567890";
	addr = bdb.put(data2, strlen(data2), addr);
	printf("\n==== append 40 bytes ====\n");
	printf("append \"%s\" after \"%s\"\n", data2, data);
	printf("should: 00000001\n");
	printf("result: %08x\n", addr);	
	
	// prepend
	char const *data3 = "yang";
	addr = bdb.put(data3, 4, addr, 0);
	
	// insert
	char const *data4 = " made";
	addr = bdb.put(data4, 5, addr, 8);
	
	std::string should;
	should = data3;
	should += data;
	should += data4;
	should += data2;

	// read loop
	char buf[4];
	std::string rec;
	size_t off(0), readCnt(0);
	while(0 < (readCnt = bdb.get(buf, 4, addr, off))){
		rec.append(buf, readCnt);
		off += readCnt;
	}
	printf("\n==== insertion then read====\n");
	printf("prepend \"yang\" at 0, insert \" made\" at 8, and read result into a c-string\n");
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());
	
	// read into string
	rec.clear();
	rec.reserve(off);
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
	addr2 = bdb.put("small", 4);
	should = "123456789012345678901234567890tail";
	bdb.update(should.c_str(), should.size(), addr2);
	bdb.get(&rec, 1024, addr2);
	printf("\n==== replace data with long value to cause migration ====\n");
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());
	

	// erase all
	bdb.del(addr);
	size_t negtive = bdb.get(&rec, 1024, addr);
	printf("\n==== del whole data ====\n");
	printf("should: 0\n");
	printf("result: %d\n", negtive);
	bdb.del(addr2);
	negtive = bdb.get(&rec, 1024, addr2);
	printf("should: 0\n");
	printf("result: %d\n", negtive);
	

	// put new data for test iterator
	AddrType addrs[3];
	addrs[0] = bdb.put("acer", 4); // should be placed in 0000.pool
	addrs[1] = bdb.put("123456789012345678901234567890", 30); // should be placed in 0001.pool
	addrs[2] = bdb.put("123456789012345678901234567890123456789012345678901234567890", 60); // should be placed in 0002.pool
	
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
	printf("should: %08x\n", 7u);
	printf("result: %08x\n", addr);
	printf("should: %s\n", "toma");
	printf("result: %s\n", rec.c_str());
	bdb.del(addr);
	return 0;	
}
