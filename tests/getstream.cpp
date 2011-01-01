#include "bdb.h"
#include <iostream>
#include <cstring>


//! \include getstream.cpp

int main()
{
	using namespace std;
	char const *data = "12345678901234567890";
	char buf[11]={0};
	
	BehaviorDB bdb;

	AddrType addr = bdb.put(data, strlen(data));
	StreamState ss;
	
	SizeType rt;
	rt = bdb.get(buf, 10, addr, &ss);
	
	// bdb is locked due to unfinished get
	if(-1 == bdb.put(data, strlen(data))){
		// Thus, we stop it explicitly
		bdb.stop_get(&ss);	
	}
	
	// This put will success
	bdb.put(data, strlen(data));

	return 0;	
}
