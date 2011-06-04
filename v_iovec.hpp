#ifndef V_IOVEC_HPP
#define V_IOVEC_HPP

#include "boost/variant.hpp"
#include <cstdio>

namespace BDB {


	struct file_src
	{
		FILE *fp;
		off_t off;
	};

	struct viov
	{
		boost::variant<file_src, char const*> data;
		size_t size;
	};

	struct write_viov : public boost::static_visitor<size_t>
	{
		size_t 
		operator()(char const* str);
		
		size_t
		operator()(file_src & psrc);

		FILE* dest;
		char *buf;
		size_t bsize;
		size_t size;
	};
	

} // end of namespace BDB

#endif
