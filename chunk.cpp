#include "chunk.h"

#include <cstdlib>
#include <cstring>
#include <istream>
#include <sstream>
#include <ostream>
#include <iomanip>

using std::hex;


std::istream& 
operator>>(std::istream &is, ChunkHeader &ch)
{
	static char buf[9];
	buf[8] = 0;

	is.read(buf, 8);
	ch.liveness = *buf;
	ch.size = strtoul(&buf[1], 0, 16);

	return is;	
}

std::ostream& 
operator<<(std::ostream &os, ChunkHeader const &ch)
{
	
	using std::ios;
	using std::setfill;
	using std::setw;
	
	os.write((char*)&ch.liveness, 1);

	ios::fmtflags oldflag = os.flags();
	os.unsetf(oldflag);
	os<<setfill('0')<<setw(7)<<hex<<ch.size;
	os.flags(oldflag);
	
	return os;	
}

