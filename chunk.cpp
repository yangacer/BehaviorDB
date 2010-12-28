#include "chunk.h"

#include <istream>
#include <ostream>
#include <iomanip>

using std::ios;
using std::hex;
using std::setfill;
using std::setw;

std::istream& 
operator>>(std::istream &is, ChunkHeader &ch)
{
	char buf[8] = {0};

	is.read(&ch.liveness, 1);
	is.read(buf, 7);
	ch.size = strtoul(buf, 0, 16);
	return is;	
}

std::ostream& 
operator<<(std::ostream &os, ChunkHeader const &ch)
{
	os.write(&ch.liveness, 1);

	ios::fmtflags oldflag = os.flags();
	os<<setfill('0')<<setw(7)<<hex<<ch.size;
	os.flags(oldflag);

	return os;	
}

