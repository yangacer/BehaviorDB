#include "poolImpl.hpp"
#include <string>

enum {
	EMPTY_CTOR = 0,
	WRT_ZERO
};

char const *TestCase[4] = {
	"empty_ctor",
	"wrt_zero"
};

int main(int argc, char** argv)
{
	using namespace BDB;
	using namespace std;

	if(argc < 2) return 0;
	
	string opt(argv[1]);

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
	}

	return 0;
}
