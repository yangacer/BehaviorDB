#include "poolImpl.hpp"
#include <string>
#include <cstdio>

enum {
	EMPTY_CTOR = 0,
	WRT_ZERO,
	WRT_S,
	WRT_PRE,
	WRT_APP,
	WRT_INS,
	READ_ZERO,
	READ_ALL,
	WRT_MIG,
	ERASE_ALL,
	ERASE_PART,
	ERR_CHK,
	R_LOOP
};

char const *TestCase[15] = {
	"empty_ctor",
	"wrt_zero",
	"wrt_simple",
	"wrt_prepend",
	"wrt_append",
	"wrt_insert",
	"read_zero",
	"read_all",
	"wrt_migrate",
	"erase_all",
	"erase_partial",
	"error_check",
	"read_loop",
	0
};

void usage()
{
	printf("./pool test_cases\n");
	printf("Available test cases:\n");
	int i=0;
	while(TestCase[i]){
		printf("\t%s\n", TestCase[i]);
		++i;	
	}
}

void
configuration()
{
	printf("minimum chunk size: 32 bytes\n");
	printf("migration certerion: chunk overflow\n");
	printf("pool count: 16\n");
	printf("chunk size groth: 2 times lager than previous\n");
}

int main(int argc, char** argv)
{
	using namespace BDB;
	using namespace std;

	if(argc < 2){ usage(); return 0;}
	
	string opt(argv[1]);
	
	if(opt == "--config"){
		configuration();
		return 0;
	}

	if(opt == TestCase[EMPTY_CTOR]){
		// default ctor test
		pool::config conf;
		pool p(conf);
	}else if(opt == TestCase[WRT_ZERO]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		AddrType rt = p.write((char*)0,0);
		printf("%08x\n", rt);
	}else if(opt == TestCase[WRT_S]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "acer";
		AddrType rt = p.write(data,4);
		printf("%08x\n", rt);
	}else if(opt == TestCase[WRT_APP]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "acer";
		AddrType rt = p.write(data,4);
		printf("%08x\n", rt);
		rt = p.merge_move(".yang", 5, rt, 4, &p);
		printf("%08x\n", rt);
	}else if(opt == TestCase[WRT_PRE]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "acer";
		AddrType rt = p.write(data,4);
		printf("%08x\n", rt);
		rt = p.merge_move("I'm ", 4, rt, 0, &p);
		printf("%08x\n", rt);
	}else if(opt == TestCase[WRT_INS]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "aceryang";
		AddrType rt = p.write(data,8);
		printf("%08x\n", rt);
		rt = p.merge_move(" is ", 4, rt, 4, &p);
		printf("%08x\n", rt);
	}else if(opt == TestCase[READ_ZERO]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		size_t rt = p.read((char*)0, 0, 0);
	}else if(opt == TestCase[READ_ALL]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		AddrType addr = p.write("acer", 4);
		printf("%08x\n", addr);
		char buf[5];
		size_t rt = p.read(buf, 5, addr);
		fwrite(buf, 1, rt, stdout);
		printf("\n");
	}else if(opt == TestCase[WRT_MIG]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		conf.dirID = 1;
		pool p2(conf);

		AddrType addr = p.write("acer", 4);
		printf("%08x\n", addr);
		char const *data = "this is a string that longer than 32 bytes";
		AddrType addr2 = p.merge_move(data, strlen(data), addr, 4, &p2);
		printf("%08x\n", addr2);
	}else if(opt == TestCase[ERASE_ALL]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		AddrType addr = p.write("acer", 4);
		printf("%08x\n", addr);
		p.erase(addr);
		char data[5];
		size_t rt = p.read(data, 4, addr);
		printf("%08x\n", rt);
	}else if(opt == TestCase[ERR_CHK]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);

		// Write too large
		char const *data = "this is a string that longer than 32 bytes";
		AddrType addr = p.write(data, strlen(data));
		printf("Should be ffffffff and actually is: %08x\n", addr);

		// Write to non-exist
		addr = p.write("ok", 2);
		size_t rt = p.erase(addr);
		addr = p.write("ok", 2, addr);
		printf("Should be ffffffff and actually is: %08x\n", addr);

		// Read from non-exist
		char buf[10];
		rt = p.read(buf, 10, addr);
		printf("Should be ffffffff and actually is: %08x\n", rt);
	}else if(opt == TestCase[R_LOOP]){
		addr_eval<AddrType> addrEval(4, 32);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "this is a string\n";
		AddrType addr = p.write(data, strlen(data));
		char buf[4];
		size_t toRead = strlen(data), readCnt, loopOff(0);
		while(toRead){
			readCnt =( toRead > 4 ) ? 4 : toRead;
			if(readCnt != p.read(buf, readCnt, addr, loopOff)){
				printf("loop read failed\n");
				break;
			}
			fwrite(buf, 1, readCnt, stdout);
			toRead -= readCnt;
			loopOff += readCnt;
		}
		if(loopOff != strlen(data))
			printf("loop read failed\n");
	}

	
	else{
		usage();

	}


	return 0;
}
