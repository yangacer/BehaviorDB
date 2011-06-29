#include "bdb.hpp"
#include <cstdio>
#include <cstring>
#include <string>

int main()
{
	using namespace BDB;
	
	Config conf;
	
	conf.min_size = 32;
	BehaviorDB bdb(conf);

	// write
	char const* data = "acer";
	AddrType addr = bdb.put(data, 4);
	printf("should: 00000001\n");
	printf("result: %08x\n", addr);	
	
	// append include migration
	char const *data2 = "1234567890asdfghjkl;12345678901234567890";
	addr = bdb.put(data2, strlen(data2), addr);
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
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());
	
	// read into string
	rec.clear();
	rec.reserve(off);
	bdb.get(&rec, 1024, addr);
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());

	// erase partial
	bdb.del(addr, 13, 10);
	bdb.get(&rec, 1024, addr);
	should.erase(13, 10);
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());
	
	// update
	bdb.update("replaced", addr);
	bdb.get(&rec, 1024, addr);
	should = "replaced";
	printf("should:\t%s\n", should.c_str());
	printf("result:\t%s\n", rec.c_str());

	// erase all
	bdb.del(addr);
	size_t negtive = bdb.get(&rec, 1024, addr);
	printf("%d\n", negtive == 0);
	return 0;	
}
