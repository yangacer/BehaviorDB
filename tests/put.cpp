#include "bdb.h"
#include <iostream>
#include <iomanip>

int main(int argc, char** argv)
{
	using namespace std;
	if(argc < 2)
		exit(1);
	
	Config conf;
	conf.pool_log = false;

	BehaviorDB bdb(conf);

	int upperbound = atoi(argv[1]);
	char data_128[129] = "128b begin";

	strcpy(data_128+128 - 7, "128b end");
	

	for(int i=0;i<upperbound; ++i){
		cout<<setw(8)<<setfill('0')<<hex<<bdb.put(data_128, 128)<<endl;
		cout.flush();
		
	}

	exit(0);
	return 0;
}
