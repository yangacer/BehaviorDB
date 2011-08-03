#include "bdb.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

#define KB *1024
#define MB *1048576

int main(int argc, char **argv)
{
	using namespace std;
	using namespace BDB;

	ifstream fin;
	size_t const bufsize = 1 MB;
	char buf[bufsize];
	Config conf;
	conf.min_size = 1 KB;
	conf.beg = 1; conf.end = 1 + 5000000;
	BehaviorDB bdb(conf);

	fin.rdbuf()->pubsetbuf(buf, bufsize);
	fin.open(argv[1], ios::in | ios::binary);

	if(!fin.is_open())
		return 0;

	string cmd;
	unsigned int size, offset;
	AddrType addr;
	vector<char> data_src(4 KB);
	
	size_t lineNum = 1;
	while(fin>>cmd){
		if("put" == cmd){
			fin>>size;
			if(size > data_src.size())
				data_src.resize(size);
			if(-1 == bdb.put(&*data_src.begin(), size)){
				cerr<<"put error at line: "<<lineNum<<endl;
			}
		}
		lineNum++;
	}

	fin.close();
	return 0;
}
