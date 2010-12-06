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
	
	SizeType 
	get(char **output, AddrType address);
private:
	// copy, assignment
	BehaviorDB(BehaviorDB const &cp);
	BehaviorDB& operator = (BehaviorDB const &cp);

	Pool* pools_;
	std::ofstream *accLog_;
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
	create_chunk_file(SizeType size);
	
	SizeType 
	chunk_size() const;

	AddrType 
	put(char const* data, SizeType size);
	
	//AddrType 
	//append(AddrType address, char const* data, SizeType size);
	
	SizeType 
	get(char **output, AddrType address);
	
	// TODO: error report mechanism
private:
	SizeType chunk_size_;
	std::fstream file_;
	IDPool<AddrType> idPool_;
	std::ofstream wrtLog_;
	
};


// ------------- BehaviorDB impl -----------------

using std::setw;
using std::hex;
using std::setfill;
using std::endl;


BehaviorDB::BehaviorDB()
: pools_(new Pool[16]), accLog_(new std::ofstream)
{
	using std::ios;

	for(SizeType i=0;i<16;++i){
		pools_[i].create_chunk_file((1<<i)<<10);	
	}

	// open access log
	accLog_->open("access.log", ios::out | ios::app);
	if(!accLog_->is_open()){
		accLog_->open("access.log", ios::out | ios::trunc);
		assert(true == accLog_->is_open());
	}
}

BehaviorDB::~BehaviorDB()
{
	accLog_->close();
	delete accLog_;
	delete [] pools_;
}

AddrType
BehaviorDB::put(char const* data, SizeType size)
{
	// Preserve 8 bytes for size value
	size+=8;

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
	
	// write access log
	*accLog_<<"[put   ] data_size(KB): "<<
		setfill(' ')<<setw(12)<<bound<<
		" address: "<<
		hex<<setw(8)<<setfill('0')<<pIdx<<
		endl;

	return pIdx;
}

/*
AddrType
BehaviorDB::append(AddrType address, char const* data, SizeType size)
{
	
}
*/

SizeType
BehaviorDB::get(char **output, AddrType address)
{
	// check address roughly
	if(address>>28 > 15){
		return 0;
	}

	return pools_[address>>28].get(output, address);
}


// ------------- Pool implementation ------------

Pool::Pool()
: idPool_(0, 1<<28)
{}

Pool::~Pool()
{
	wrtLog_.close();
	file_.close();
}

void
Pool::create_chunk_file(SizeType chunk_size)
{ 
	using namespace std;

	chunk_size_ = chunk_size;

	if(file_.is_open())
		file_.close();
	
	stringstream cvt;
	
	cvt<<"pools/"
		<<setw(4)<<setfill('0')<<hex
		<< (chunk_size_>>10)
		<< ".pool";
	{
		// create chunk file
		char const* name(cvt.str().c_str());
		file_.open(name, ios_base::in | ios_base::out);
		if(!file_.is_open()){
			file_.open(name, ios_base::in | ios_base::out | ios_base::trunc);
			assert(file_.is_open() == true);
		}
	}
	
	cvt<<".log";
	{
		char const *name(cvt.str().c_str());
		// init write log
		wrtLog_.open(name, ios_base::out | ios_base::app);
		if(!wrtLog_.is_open()){
			wrtLog_.open(name, ios_base::out | ios_base::trunc);
			assert(wrtLog_.is_open() == true);
		}
		wrtLog_<<unitbuf;
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
	
	AddrType off = idPool_.Acquire();
	
	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	wrtLog_<<"off(KB): "<<setw(8)<<setfill('0')<<off<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<endl;

	// write 8 bytes size value ahead
	file_<<setw(8)<<setfill('0')<<size;
	file_.write(data, size);
	
	if(!file_){
		return 0;	
	}

	
	return off;
}


SizeType 
Pool::get(char **output, AddrType address)
{
	using namespace std;
	if(*output){
		delete [] *output;	// !!! segmentation fault when 
					// *output is not initialized
		*output = 0;
	}
	
	// TODO: check wheather the address has record
	// Following code assume size value > 0
	
	char size_val_ar[9] = {0};
	char *size_val(size_val_ar);

	AddrType off = (address & 0x0fffffff) * chunk_size_;
	
	file_.seekg(off, ios::beg);
	file_.read(size_val, 8);
	
	while('0' == *size_val)
		++size_val;
	
	SizeType size(strtoul(size_val, 0, 10));
	*output = new char[size];
	file_.read(*output, size);

	if(!file_.gcount()){ // read failure
		delete [] *output;
		*output = 0;
		return 0;
	}
	
	return size;

}


// test main
#include <iostream>
#include <iomanip>
int main(int argc, char **argv)
{
	using namespace std;

	BehaviorDB bdb;
	bdb.put(argv[1], atoi(argv[2]));
	bdb.put(argv[1], atoi(argv[2]));
	bdb.put(argv[1], atoi(argv[2]));
	return 0;	
}
