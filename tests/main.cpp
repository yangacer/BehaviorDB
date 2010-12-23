
// test main
#include <iostream>
#include <iomanip>
#include "bdb.h"

#define MB32 ((1<<25)-8)

int main(int argc, char **argv)
{
	using namespace std;

	Config conf;
	conf.pool_log = true;
	conf.chunk_unit = 10;

	BehaviorDB bdb(conf);
	AddrType addr;
	
	char *data = new char[MB32];
	
	memcpy(data, "32M data begin", 15);
	
	memcpy(data + (MB32 - 6), "tail", 5);

	for(int i=0;i<(1<<8);++i){
		addr = bdb.put(data, MB32);
		if(addr == -1 && bdb.error_num){
			cerr<<"Fail at "<<i<<" data"<<endl;
			return 0;
		}
		cout<<setw(8)<<hex<<setfill('0')<<addr<<endl;
	}

	
	return 0;	
}
