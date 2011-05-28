#include "utils.hpp"

char error_num_to_str::buf[6][40] = {
	"No error",
	"Address overflow",
	"System error",
	"Data too big",
	"Pool locked",
	"Non exist address"
};

/*
unsigned int
MonitorPred(refHistory const& rh, AddrType address, unsigned int next_pool_idx)
{
	countResult cr = rh.count(address);
	printf("@\nPUT: %.3f\nAPPEND: %.3f\nGET: %.3f\n",
		100 * cr.count[0] / (double)rh.size(),
		100 * cr.count[1] / (double)rh.size(),
		100 * cr.count[2] / (double)rh.size()
	);

	return next_pool_idx;
}
*/
