#include "bdb.hpp"
#include <iostream>
#include <iomanip>

void usage()
{
	printf("put amount\n");	
}

int main(int argc, char** argv)
{
	using namespace std;
	using namespace BDB;

	Config conf;
	conf.root_dir = "tmp/";

	conf.min_size = 32;
	BehaviorDB bdb(conf);
	
	char const* data = "acer";
	AddrType addr = bdb.put(data, 4);
	
	return 0;
}
