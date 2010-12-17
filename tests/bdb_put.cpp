
// test main
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include "bdb.h"

#define BUFFER_SIZ 1024 * 1024

int main(int argc, char **argv)
{
	using namespace std;

	Config conf;
	conf.pool_log = false;
	conf.chunk_unit = 9;

	BehaviorDB bdb(conf);
	AddrType addr1, addr2;
	SizeType rt;

	FILE *fp = fopen(argv[1], "r");
	char filebuf[BUFFER_SIZ]; // buffer for file stream
	char buffer[BUFFER_SIZ]; //1M
	int i;
	setvbuf(fp, filebuf, _IOFBF, BUFFER_SIZ);

	i = 0;
	while (fgets(buffer, BUFFER_SIZ, fp)) {
		/*
		if (++i % 1000 == 0) {
			printf("#recs = %d\n", i);
		}
		*/
		bdb.put(buffer, strlen(buffer));
	}

	return 0;
}
