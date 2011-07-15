#include "bdb.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>

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
	vector<AddrType> addrs;
	for(int i=0;i<10000;++i){
		addrs.push_back(bdb.put(data, 4));
		if(-1 == addrs[i]){
			cerr<<"put failure"<<endl;
			break;
		}
		printf("%08x\n", addrs[i]);
	}
	
	for(int i=0;i<10000;++i){
		bdb.del(addrs[i]);
	}

	return 0;
}
