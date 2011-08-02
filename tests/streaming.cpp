#include "bdb.hpp"
#include <iostream>
#include <cstdio>


int main(int argc, char** argv)
{
	using namespace std;
	using namespace BDB;

	Config conf;
	conf.root_dir = argv[1];

	BehaviorDB bdb(conf);
	char const* data("acer");
	char buf[20] = {};

	// simulate asynchronous operation
	stream_state const *reader, *writer;
	
	AddrType addr = bdb.put(data, 4);
	
	// read partial
	reader = bdb.istream(4, addr, 0);
	reader = bdb.stream_read(reader, buf, 2);

	// write partial
	writer = bdb.ostream(4, addr, 0);
	writer = bdb.stream_write(writer, "ya", 2);
		
	// write finish
	writer = bdb.stream_write(writer, "ng", 2);
	bdb.stream_finish(writer);
	
	// read finish
	reader = bdb.stream_read(reader, buf+2, 2);
	bdb.stream_finish(reader);

	printf("should: acer\n");
	printf("result: %s\n", buf);

	// verify written data
	bdb.get(buf, 20, addr, 0);

	printf("should: yangacer\n");
	printf("result: %s\n", buf);

	return 0;
}
