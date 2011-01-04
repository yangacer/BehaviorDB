#include "bdb.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>

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

	int n = atoi(argv[1]);
	double x = n / 53.93232738911;

	int dist[16] = {};
	int cnt;
	for(int i=0;i<8;++i){
		cnt = x*pow(1.333333333333, i);
		dist[i] = dist[15-i] = cnt;
	}
	/*
	for(int i=0;i<15;++i){
		cout<<dist[i]<<"\n";	
	}
	*/

	char data[129];

	create("128 head", data, 128);
	
	Config conf;
	conf.migrate_threshold = 0x40;
	BehaviorDB bdb(conf);
	AddrType addr, tmp;
	SizeType size;
	string line;
	
	for(int i=1;i<16;++i){
		for(int j=0;j<dist[i];++j){
			
			getline(cin,line);
			cout<<line<<"\n";
			if(!cin)
				return 1;
			addr = strtoul(line.c_str(), 0, 16);
			cout<<addr<<"\n";
			line.clear();
			while(addr>>28 < i && addr != -1){ 
				addr = bdb.append(addr, data, 128);
				
			}
			if(addr == -1){
				cout<<"error encountered\n";
				return 1;
			}
		}
	}

	cout<<"Finished\n";

	return 0;
}
