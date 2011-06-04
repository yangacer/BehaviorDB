#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <iosfwd>
#include <istream>
#include <cstdio>

struct ChunkHeader
{
	size_t size;
	
	ChunkHeader()
	:size(0)
	{}
};

std::istream& 
operator>>(std::istream& is, ChunkHeader & ch);

std::ostream& 
operator<<(std::ostream& os, ChunkHeader const& ch);

FILE*
operator>>(FILE* fp, ChunkHeader &ch);

FILE*
operator<<(FILE* fp, ChunkHeader const &ch);

int
write_header(FILE* fp, ChunkHeader const &ch);

int
read_header(FILE* fp, ChunkHeader &ch);

#endif

