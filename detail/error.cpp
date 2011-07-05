#include "error.hpp"

namespace BDB {

	char error_num_to_str::buf[6][40] = {
		"No error",
		"Address overflow",
		"System error",
		"Data too big",
		"Pool locked",
		"Non exist address"
	};

} // end of namespace BDB
