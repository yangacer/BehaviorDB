#include <iostream>
#include <iomanip>
#include "bdb.h"

int main()
{
	using namespace std;

	char const data1[1024] = "1k";
	char const data2[2048] = "2k";

	BehaviorDB bdb;
	bdb.put(data1, 1024);
	bdb.put(data2, 2048);

	AddrIterator iter = bdb.begin();
	for(; iter != bdb.end(); ++iter)
		cout<<hex<<*iter<<endl;

	/*
	bdb.del(2);
	
	iter = bdb.begin();
	for(; iter != bdb.end(); ++iter)
		cout<<hex<<*iter<<endl;
	*/

}
