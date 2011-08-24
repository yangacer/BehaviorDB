#include "bdb.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define KB *1024
#define MB *1048576

void usage()
{ 
	using namespace std;
	cout<<"./simulator -w work_dir -h header_dir workload"<<endl;
	exit(1);
}

int main(int argc, char **argv)
{
	using namespace std;
	using namespace BDB;
		
	if(argc < 2)
		usage();

	char const *work_dir = 0;
	char const *header_dir = 0; 
	char const *workload = 0;

	for(int i=1;i<argc;++i){
		if(0 == strcmp("-w", argv[i])){
			if(i+1 >= argc) usage();
			work_dir = argv[i+1];
			++i;
		}else if(0 == strcmp("-h", argv[i])){
			if(i+1 >= argc) usage();
			header_dir = argv[i+1];
			++i;
		}else {
			workload = argv[i];	
		}
		
	}

	ifstream fin;
	size_t const bufsize = 1 MB;
	char buf[bufsize];
	Config conf;
	conf.min_size = 1 KB;
	if(work_dir) conf.root_dir = work_dir;
	if(header_dir) conf.header_dir = header_dir;
	conf.beg = 1; conf.end = 1 + 5000000;
	BehaviorDB bdb(conf);

	fin.rdbuf()->pubsetbuf(buf, bufsize);
	fin.open(workload, ios::in | ios::binary);

	if(!fin.is_open())
		return 0;

	string cmd;
	string arg;
	size_t size, offset;
	AddrType addr;
	string data_src;
	string data_dest;
	data_src.resize(4 KB);
	data_dest.resize(4 KB);

	size_t lineNum = 1;
	while(fin>>cmd){
		if("put" == cmd){
			fin>>arg;
			size = strtoul(arg.c_str(), 0, 16);
			if(size > data_src.size())
				data_src.resize(size);
			if(-1 == bdb.put(data_src.c_str(), size)){
				cerr<<"put error at line: "<<lineNum<<endl;
			}
		}else if("get" == cmd){
			fin>>arg;
			size = strtoul(arg.c_str(), 0, 16);
			fin>>arg;
			addr = strtoul(arg.c_str(), 0, 16);
			fin>>arg;
			offset = strtoul(arg.c_str(), 0, 16);
			if(size > data_dest.size()) 
				data_dest.resize(size);
 			if(-1 == bdb.get(&data_dest, size, addr, offset))
				cerr<<"get error at line: "<<lineNum<<endl;
		}
		lineNum++;
	}

	fin.close();
	return 0;
}
