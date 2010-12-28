#include "chunk.h"

#include <iostream>
#include <sstream>

int main()
{
	using namespace std;

	stringstream ss;
	ChunkHeader ch, ch2;
	
	ch.liveness = 'a';
	ch.size = 127;
	
	ss<<ch;

	cout<<ss.str()<<endl;
	ss>>ch2;
	
	ss.str("");
	ss.clear();

	ss<<ch2;
	cout<<ss.str()<<endl;
	return 0;	
}
