#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <iosfwd>
#include "bdb.h"

struct ChunkHeader
{
	char liveness;
	SizeType size;
	
	ChunkHeader()
	:liveness(0), size(0)
	{}
};

std::istream& 
operator>>(std::istream& is, ChunkHeader & ch);

std::ostream& 
operator<<(std::ostream& os, ChunkHeader const& ch);

#endif

