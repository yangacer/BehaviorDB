/*
 * Test estimate_pool_index function
 * 
 * Output Example:
 *			Size	Pool Index
 * Before boundary	1023:		 0
 * On boundary		1024:		 0
 * After boundary	1025:		 1
 *
 */

#include <iostream>
#include <iomanip>

typedef unsigned int AddrType;
typedef unsigned int SizeType;

inline AddrType 
estimate_pool_index(SizeType size)
{
	// Determin which pool to put
	AddrType pIdx(0);
	SizeType bound(size); // +8 for size value
	
	while(bound > (1<<pIdx)<<10)
		++pIdx;
	
	return pIdx;
}

int main()
{
	using namespace std;

	SizeType i(0), tmp;

	for(; i<16; ++i){
		tmp = (1<<i)<<10;
		cout<<"Before boundary\t"<<setw(12)<<(tmp-1)<<":"<<setw(12)<<estimate_pool_index(tmp-1)<<endl;
		cout<<"On boundary\t"<<setw(12)<<(tmp)<<":"<<setw(12)<<estimate_pool_index(tmp)<<endl;
		cout<<"After boundary\t"<<setw(12)<<(tmp+1)<<":"<<setw(12)<<estimate_pool_index(tmp+1)<<endl;
		cout<<endl;
	}

	return 0;	
}
