
// test main
#include <iostream>
#include <iomanip>

#include "bdb.h"
int main(int argc, char **argv)
{
	using namespace std;

	BehaviorDB bdb;
	AddrType addr1, addr2;

	char two_kb[2048] = "2k_data_tailed_by_null";
	char ten_kb[10240] = "10k_data_tailed_by_null";
	
	addr1 = bdb.put(two_kb, 2048);
	addr2 = bdb.append(addr1, ten_kb, 10240);

	return 0;	
}
