#include "v_iovec.hpp"

namespace BDB {

	size_t
	write_viov::operator()(file_src &fsrc)
	{
		if(-1 == fseeko(fsrc.fp, fsrc.off, SEEK_SET))
			return 0;

		size_t toRead = size;
		size_t readCnt;
		while(toRead){
			readCnt = (bsize > toRead) ? toRead : bsize;
			if(readCnt != fread(buf, 1, readCnt, fsrc.fp))
				return 0;
			if(readCnt != fwrite(buf, 1, readCnt, dest))
				return 0;
			toRead -= readCnt;
		}
		return size;
	}
	
	size_t
	write_viov::operator()(char const* str)
	{
		if(size != fwrite(str, 1, size, dest))
			return 0;
		return size;
	}

} // end of namespace BDB
