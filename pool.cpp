#include <iosfwd>

typedef unsigned int AddrType;
typedef unsigned int SizeType;

struct Pool;

struct BehaviorDB
{
	
	BehaviorDB();
	~BehaviorDB();

	AddrType 
	put(char const* data, SizeType size);
	
	//AddrType 
	//append(AddrType address, char const* data, SizeType size);
	
	//SizeType 
	//get(char **output, AddrType address);
private:
	// copy, assignment
	BehaviorDB(BehaviorDB const &cp);
	BehaviorDB& operator = (BehaviorDB const &cp);

	Pool* pools_;

};

// header ends

// src begin

#include <fstream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include "idPool.h"

#include <iostream>

struct Pool
{
	Pool();
	~Pool();

	void 
	chunk_size(SizeType size);
	
	SizeType 
	chunk_size() const;

	AddrType 
	put(char const* data, SizeType size);
	
	//AddrType 
	//append(AddrType address, char const* data, SizeType size);
	
	//SizeType 
	//get(char **output, AddrType address);
	
	// TODO: error report mechanism
private:
	SizeType chunk_size_;
	std::fstream file_;
	IDPool<AddrType> idPool_;
	
};

// ------------- BehaviorDB impl -----------------

BehaviorDB::BehaviorDB()
: pools_(new Pool[16])
{
	for(SizeType i=0;i<16;++i){
		pools_[i].chunk_size((1<<i)<<10);	
	}	
}

BehaviorDB::~BehaviorDB()
{
	delete [] pools_;
}

AddrType
BehaviorDB::put(char const* data, SizeType size)
{
	// Determin which pool to put
	AddrType pIdx(0);
	SizeType i(1), 
		bound(size>>10);
	
	if(bound<<10 < size)
		++bound;
	
	if(bound > 1<<15) // exceed capacity
		return 0;

	while(bound > (i<<=1))
		++pIdx;
	
	pIdx = pIdx<<28 | pools_[pIdx].put(data, size);
	
	return pIdx;
}

/*
AddrType
BehaviorDB::append(AddrType address, char const* data, SizeType size)
{
	
}


SizeType
BehaviorDB::get(char **output, AddrType address)
{
	if(address>>28 > 15){
		return 0;
	}

	return pools_[address>>28].get(output, address);
}
*/

// ------------- Pool implementation ------------

Pool::Pool()
: idPool_(0, 1<<28)
{}

Pool::~Pool()
{
	file_.close();
}

void
Pool::chunk_size(SizeType chunk_size)
{ 
	using namespace std;

	chunk_size_ = chunk_size;

	if(file_.is_open())
		file_.close();
	
	stringstream cvt;
	cvt<<"pool/"
		<<setw(4)<<setfill('0')<<hex
		<< (chunk_size_>>10)
		<< ".pool";
	
	char const* name(cvt.str().c_str());
	
	cout<<name<<endl;
	file_.open(name, ios_base::in | ios_base::out);

	if(!file_.is_open()){
		file_.open(name, ios_base::in | ios_base::out | ios_base::trunc);
		assert(file_.is_open() == true);
	}
	
	return;
}

SizeType
Pool::chunk_size() const
{ return chunk_size_; }

AddrType
Pool::put(char const* data, SizeType size)
{
	using namespace std;

	if(!idPool_.avail()){
		return 0; // temp used
	}
	
	AddrType off = idPool_.Acquire() * chunk_size_;
	
	file_.clear();
	file_.seekp(off, ios_base::beg);
	
	cout<<" off: "<<off<<" tellp: "<<file_.tellp()<<endl;

	// write 8 bytes size value ahead
	file_<<setw(8)<<setfill('0')<<size;
	file_.write(data, size);
	
	if(!file_){
		return 0;	
	}

	return off;
}

/*
SizeType 
Pool::get(char **output, AddrType address)
{
	if(*output){
		delete [] *output;	// segmentation fault when *output 
					// is not initialized
		*output = 0;
	}

	// ...
}
*/

// test main
#include <iostream>
#include <iomanip>
int main(int argc, char **argv)
{
	using namespace std;

	BehaviorDB bdb;
	cout<<"return: "<<bdb.put(argv[1], atoi(argv[2]));
	cout<<"return: "<<bdb.put(argv[1], atoi(argv[2]));
	bdb.put(argv[1], atoi(argv[2]));
	//cout<<setw(8)<<bdb.put(argv[1], atoi(argv[2]))<<endl;;
	return 0;	
}
