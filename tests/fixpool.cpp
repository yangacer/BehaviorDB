#include "fixedPool.hpp"
#include "chunk.h"

int main()
{
	using namespace BDB;

	ChunkHeader header;
	header.size = 17;
	
	fixed_pool<ChunkHeader, 8> fpool(0, "");
	
	fpool.write(header, 0);

	header.size = 18;
	fpool.write(header, 1);
	
	fpool.read(&header, 0);

	printf("%08x\n", header.size);

	fpool.read(&header, 1);

	printf("%08x\n", header.size);
}
