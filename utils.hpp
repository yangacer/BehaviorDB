#ifndef BDB_UTILS_HPP
#define BDB_UTILS_HPP

#include "refHistory.h"

struct error_num_to_str
{
	char const *operator()(int error_num)
	{
		return buf[error_num];
	}
private:
	static char buf[6][40];
};

/*
unsigned int
MonitorPred(refHistory const& rh, AddrType address, unsigned int next_pool_idx);
*/

#endif // end of header guard
