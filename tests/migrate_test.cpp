#include "bdb.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

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

 /** This test should output
 put     Addr:        0 Head: 128 head::Tail: data end
error occured
Pool[0]:       0
Pool[1]:       0
Pool[2]:       1
Pool[3]:       2
Pool[4]:       4
Pool[5]:       8
Pool[6]:      16
Pool[7]:      32
Pool[8]:      64
Pool[9]:     128
Pool[10]:     256
Pool[11]:     512
Pool[12]:    1024
Pool[13]:    2048
Pool[14]:    4096
Pool[15]:    8192

The only error will occur is ADDRESS_OVERFLOW

*/
int main()
{

	int dist[16] = {0};
	char data[129];
	char data_2k[2049];

	create("128 head", data, 128);
	create("2k head", data_2k, 2048);
	
	Config conf;
	conf.pool_log = true;
	conf.migrate_threshold = 0x40;

	BehaviorDB bdb(conf);
	AddrType addr;
	SizeType size;

	addr = bdb.put(data, 128);
	size = bdb.get(data, 128, addr);
	printf("put\tAddr: %8x ", addr);
	verify(data, 128);
	printf("\n");
	
	while(1){
		addr = bdb.append(addr, data_2k, 2048);
		if(addr >= 0xf00000ffu) break;
		dist[addr>>28]++;
	}
	if(addr == -1){
		printf("error occured\n");	
	}
	for(int i=0;i<16;++i)
		printf("Pool[%d]:%8d\n", i, dist[i]);	
	
	return 0;
}
