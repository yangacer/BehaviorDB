#include "bdb.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cstdlib>

void usage()
{
	printf("put amount\n");	
	exit(1);
}

int main(int argc, char** argv)
{
	using namespace std;
	using namespace BDB;

	if(argc < 2) usage();

	Config conf;
	conf.root_dir = "tmp/";

	conf.min_size = 32;
	BehaviorDB bdb(conf);
	
	int amount = atoi(argv[1]);
	char const* data = "acer";
	vector<AddrType> addrs;

	for(int i=0;i<amount;++i){
		addrs.push_back(bdb.put(data, 4));
		if(-1 == addrs[i]){
			cerr<<"put failure"<<endl;
			break;
		}
		printf("%08x\n", addrs[i]);
	}
	
	/*
	for(int i=0;i<amount;++i){
		bdb.del(addrs[i]);
	}
	*/
	return 0;
}
