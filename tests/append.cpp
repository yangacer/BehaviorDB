#include "bdb.h"

//#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>

void create(char const *prefix, char *data, size_t size)
{
	strcpy(data, prefix);
	strcpy(data + size - strlen("data end"), "data end");
}

void verify(char const *data, size_t size)
{
	printf("Head: %s::", data);
	printf("Tail: %s", data+size-strlen("data end"));
}

/** \include append.cpp
 */

int main(int argc, char **argv)
{
	using std::string;
	using std::cin;
	using std::cout;
	using std::setw;
	using std::setfill;
	using std::endl;

	// timeval beg;
	// gettimeofday(&beg, 0);
	
	unsigned int addrBeg = strtoul(argv[2], 0, 16);
	int n = atoi(argv[1]);
	double x = n / 53.93232738911;

	int dist[16] = {};
	int cnt;
	for(int i=0;i<8;++i){
		cnt = x*pow(1.333333333333, i);
		dist[i] = dist[15-i] = cnt;
	}

	char data[129];

	create("128 head", data, 128);
	
	Config conf;
	conf.migrate_threshold = 0x40;
	BehaviorDB bdb(conf);
	AddrType addr, tmp;
	SizeType size;
	string line;
	
	size_t accessCnt = 0;
	for(int i=1;i<16;++i){
		for(int j=0;j<dist[i];++j){
			
			// getline(cin,line);
			// cout<<line<<"\n";
			// if(!cin)
			// 	return 1;
			//addr = strtoul(line.c_str(), 0, 16);
			//line.clear();
			//
			addr = addrBeg++;
			while(addr>>28 < i && addr != -1){ 
				accessCnt++;
				addr = bdb.append(addr, data, 128);
				
			}
			if(addr == -1){
				cout<<"error encountered\n";
				return 1;
			}
		}
	}
	/*
	timeval end;
	gettimeofday(&end, 0);

	unsigned long sec, usec;
	sec = end.tv_sec - beg.tv_sec;
	usec = end.tv_usec - beg.tv_usec;
	if(usec < 0){
		usec += 1000000;
		sec -= 1;
	
	}
	cout<<sec<<"."<<setfill('0')<<setw(6)<<usec<<endl;
	*/
	cout<<"access count: "<<accessCnt<<endl;
	return 0;
}
