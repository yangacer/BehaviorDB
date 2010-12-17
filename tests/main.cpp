
// test main
#include <iostream>
#include <iomanip>
#include "bdb.h"

int main(int argc, char **argv)
{
	using namespace std;

	Config conf;
	conf.pool_log = false;
	conf.chunk_unit = 8;

	BehaviorDB bdb(conf);
	AddrType addr1, addr2;
	SizeType rt;

	char two_kb[2040] = "2k_data_tailed_by_null";
	char ten_kb[10240] = "10k_data_tailed_by_null";
	char merge[10240+2040];

	bdb.set_pool_log(false);
	addr1 = bdb.put(two_kb, 2040);
	addr2 = bdb.append(addr1, ten_kb, 10240);
	
	rt = bdb.get(two_kb, 2040, 536870912);
	cout<<rt<<endl;

	rt = bdb.get(merge, 10240+2040, addr2);
	
	if(rt == -1 && bdb.error_num)
		cerr<<"Fail"<<endl;
	
	cout<<(&merge[0])<<(&merge[2040])<<endl;
	
	return 0;	
}
