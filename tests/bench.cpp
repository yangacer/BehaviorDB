#include "bench.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include "bdb.h"
#include "GAISUtils/profiler.h"
#include <fcntl.h>
#include <cstring>
#include <cerrno>

void usage()
{
	printf("bench distribution-file [b|f]\n");
	printf("\'b\' for BehaviorDB benchmark(default).\n");
	printf("\'f\' for filesystem benchmark.\n");
}

int main(int argc, char** argv)
{
	using namespace std;

	char mode = 'b';
	ifstream fin;
	char buffer[1024*1024];
	switch(argc){
	case 3:
		mode = argv[2][0];
		if(mode != 'b' && mode != 'f'){
			usage();
			exit(1);
		}
	case 2:
		fin.open(argv[1]);
		if(!fin.is_open()){
			cerr<<"Error: Open distribution file failed"<<endl;
			usage();
			exit(1);
		}
		fin.rdbuf()->pubsetbuf(buffer, 1024*1024);
	break;
	default:
		usage();
		return 0;
	}

	Config conf;
	conf.pool_log = false;
	conf.chunk_unit = 7;
	conf.migrate_threshold = 0x32;
	BehaviorDB bdb(conf);
	unsigned int handleCnt(0), accessCnt(0), handle(0); 
	char data[100] = "data 100";
	strcpy(data + 100 -5, "end\n");


	fin>>handleCnt>>accessCnt;
	
	timeval put, append;
	if('b' == mode){
		vector<int> handles(handleCnt);
		TimeBeg(put);
		for(int i=0; i<handleCnt; ++i){
			handles[i] = (bdb.put(data, 100));
			if(handles[i] == -1 && bdb.error_num){
				cerr<<"Put bdb failed"<<endl;
				exit(1);
			}
		}
		TimeEnd(put);
		TimeBeg(append);
		for(size_t i=0; i<accessCnt; ++i){
			fin>>handle;
			handles[handle] = bdb.append(handles[handle], data, 100);
		}
		TimeEnd(append);
	}else{ // if('f' == mode){
		vector<FILE*> handles(handleCnt, 0);
		stringstream cvt;
		TimeBeg(put);
		for(int i=0; i<handleCnt; ++i){
			cvt.str("");
			cvt<<"./benchfiles/";
			cvt<<setw(8)<<setfill('0')<<hex<<i;
			handles[i] = fopen(cvt.str().c_str(), "a");
			if(handles[i] == 0){
				cerr<<"Open written file error"<<endl;
				cerr<<"System: "<<strerror(errno)<<endl;
				exit(1);
			}
			fwrite(data, 1, 100, handles[i]);
		}
		TimeEnd(put);
		TimeBeg(append);
		for(size_t i=0; i<accessCnt; ++i){
			fin>>handle;
			fwrite(data, 1, 100, handles[handle]);
		}
		for(int i=0; i< handleCnt; ++i)
			fclose(handles[i]);
		TimeEnd(append);
	}
	cout.clear();
	Profiler.dump("Pool Append");

}
