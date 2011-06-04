#include "poolImpl.hpp"
#include <string>
#include <cstdio>

enum {
	EMPTY_CTOR = 0,
	WRT_ZERO,
	WRT_NO_MIG
};

char const *TestCase[4] = {
	"empty_ctor",
	"wrt_zero",
	"wrt_no_mig"
};

void
configuration()
{
	printf("minimum chunk size: 128 bytes\n");
	printf("migration certerion: chunk overflow\n");
	printf("pool count: 16\n");
	printf("chunk size groth: 2 times lager than previous\n");
}

int main(int argc, char** argv)
{
	using namespace BDB;
	using namespace std;

	if(argc < 2) return 0;
	
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
		addr_eval<AddrType> addrEval(4, 128);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		AddrType rt = p.write(0,0);
		printf("%08x\n", rt);
	}else if(opt == TestCase[WRT_NO_MIG]){
		addr_eval<AddrType> addrEval(4, 128);
		pool::config conf;
		conf.addrEval = &addrEval;
		pool p(conf);
		char const *data = "acer";
		AddrType rt = p.write(data,5);
		printf("%08x\n", rt);
	}

	return 0;
}
