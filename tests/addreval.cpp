#include "addr_eval.hpp"
#include <iostream>
#include <cstdio>
#include <iomanip>

int main()
{
	using namespace BDB;
	using namespace std;

	addr_eval<unsigned int> ae(4, 256);
	
	printf("directory count: %u\n", ae.dir_count());

	unsigned int i =0;
	size_t tmp;
	for(; i < ae.dir_count(); ++i){
		
		printf("dir %4u\n", i);
		tmp = ae.chunk_size_estimation(i);
		printf("\tchunk_size_estimation(i): %lu\n", tmp);
		printf("\tdirectory(size): %u\n" ,ae.directory(tmp));
		printf("\taddr_to_dir(i<<28): %u\n", ae.addr_to_dir(i<<28));
	}

	cout<<"global_addr(2, 0x00000001): "<<hex<<ae.global_addr(2, 0x00000001)<<endl;
	cout<<"global_addr(2, 0xffffffff): "<<hex<<ae.global_addr(2, -1)<<endl;
	cout<<"local_addr(0x20000001): "<<hex<<ae.local_addr(0x20000001)<<endl;
}
