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
	
	AddrType 
	append(AddrType address, char const* data, SizeType size);
	
	SizeType 
	get(char **output, AddrType address);

	AddrType
	del(AddrType address);

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
	
	AddrType 
	append(AddrType address, char const* data, SizeType size, 
		AddrType next_pool_idx, Pool* next_pool);
	
	SizeType 
	get(char **output, AddrType address);

	AddrType
	del(AddrType address);

	// TODO: error report mechanism
protected:
	
	SizeType
	sizeOf(AddrType address);

	AddrType
	migrate(std::fstream &src_file, SizeType orig_size, 
		char const *data, SizeType size); 

private:
	
	SizeType chunk_size_;
	std::fstream file_;
	IDPool<AddrType> idPool_;
	std::ofstream wrtLog_;
	
};

// ---------------- Misc Functions --------------

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

// ------------- BehaviorDB impl -----------------

using std::setw;
using std::hex;
using std::setfill;
using std::endl;
using std::ios;

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

	AddrType pIdx = estimate_pool_index(size+8);
	
	if(pIdx > 15){ // exceed capacity
		return 0;
	}
	
	pIdx = pIdx<<28 | pools_[pIdx].put(data, size);
	
	// write access log
	accLog_->unsetf(ios::hex);
	*accLog_<<"[put   ] data_size(B): "<<
		setfill(' ')<<setw(8)<<size<<
		" address: "<<
		hex<<setw(8)<<setfill('0')<<pIdx<<
		endl;

	return pIdx;
}


AddrType
BehaviorDB::append(AddrType address, char const* data, SizeType size)
{
	// check address roughly
	if(address>>28 > 15){
		return 0;
	}

	AddrType pIdx = address >> 28;
	AddrType rt, next_pIdx(0);
	
	// Estimate next pool ----------------
	// ! No preservation space for size value since
	// it had been counted as part of existed data
	next_pIdx = estimate_pool_index(size+((1<<pIdx)<<10));

	if(pIdx > 15){ //exceed capacity
		return 0;
	}

	rt = pools_[pIdx].append(address, data, size, next_pIdx, &pools_[next_pIdx]);

	// write access log
	accLog_->unsetf(ios::hex);
	*accLog_<<"[append] data_size(B): "<<
		setfill(' ')<<setw(8)<<size<<
		" address: "<<
		hex<<setw(8)<<setfill('0')<<rt<<
		endl;

	return rt;

}

SizeType
BehaviorDB::get(char **output, AddrType address)
{
	// check address roughly
	if(address>>28 > 15){
		return 0;
	}

	SizeType rt = pools_[address>>28].get(output, address);
	
	// write access log
	accLog_->unsetf(ios::hex);
	*accLog_<<"[get   ] data_size(B): "<<
		setfill(' ')<<setw(12)<<rt<<
		" address: "<<
		hex<<setw(8)<<setfill('0')<<address<<
		endl;
	return rt;
}

AddrType
BehaviorDB::del(AddrType address)
{
	// check address roughly
	if(address>>28 > 15){
		return 0;
	}

	AddrType rt = pools_[address>>28].del(address);

	// write access log
	accLog_->unsetf(ios::hex);
	*accLog_<<"[del   ] data_size(B): "<<
		setfill(' ')<<setw(12)<<"unknown"<<
		" address: "<<
		hex<<setw(8)<<setfill('0')<<address<<
		endl;

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

	cvt.clear();
	cvt.str("");
	cvt<<"transcations/"
		<<setw(4)<<setfill('0')<<hex
		<< (chunk_size_>>10)
		<<".trs";

	idPool_.replay_transcation(cvt.str().c_str());

	idPool_.init_transcation(cvt.str().c_str());

	return;
}

SizeType
Pool::chunk_size() const
{ return chunk_size_; }

SizeType
Pool::sizeOf(AddrType address)
{

	AddrType off = (address & 0x0fffffff);
	char size_val_ar[9] = {0};
	char *size_val(size_val_ar);

	file_.seekg(off * chunk_size_, ios::beg);
	file_.read(size_val, 8);
	
	if(file_.bad())
		return 0;

	while('0' == *size_val)
		++size_val;
	
	return (strtoul(size_val, 0, 10));
}

AddrType
Pool::put(char const* data, SizeType size)
{
	// TODO: Partial buffering for big chunk

	if(!idPool_.avail()){
		return 0; // temp used
	}
	
	AddrType off = idPool_.Acquire();
	
	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	
	// write 8 bytes size value ahead
	file_<<setw(8)<<setfill('0')<<(size);
	file_.write(data, (size));
	
	if(!file_){
		return 0;	
	}
	
	// write log
	wrtLog_<<"[put    ] off(KB): "<<setw(8)<<setfill('0')<<off<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<
		" write(B): "<<setw(8)<<size<<endl;

	
	return off;
}

AddrType 
Pool::append(AddrType address, char const* data, SizeType size, 
	AddrType next_pool_idx, Pool* next_pool)
{
	using std::stringstream;
	
	// TODO: check wheather the address has record
	// Following code assume size value > 0

	SizeType used_size = sizeOf(address);
	
	if(used_size + size > chunk_size_){ // needto migration
		if(0 == next_pool){ // no pool for migration
			return 0;	
		}
		AddrType rt = next_pool_idx<<28 | next_pool->migrate(file_, used_size, data, size);
		// if no error happen in migration
		idPool_.Release(address&0x0fffffff);
		return rt;
	}
	
	// update new size
	stringstream cvt;
	cvt<<setw(8)<<setfill('0')<<(used_size + size);
	
	file_.clear();
	file_.seekp(-8, ios::cur);
	file_.write(cvt.str().c_str(), 8);

	// append data
	file_.seekp(used_size, ios::cur);
	file_.write(data, size);

	if(!file_){ // write failed
		return 0;
	}

	// write log
	wrtLog_<<"[append ] off(KB): "<<setw(8)<<setfill('0')<<(address & 0x0fffffff)<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<
		" write(B): "<<setw(8)<<(used_size + size)<<endl;


	return address;		
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
	
	SizeType size(sizeOf(address));
	
	if(size == 0){ // sizeOf failure
		return 0;	
	}

	*output = new char[size];

	// assume sizeOf will seek to the data begin.
	// is that dangerous?
	file_.read(*output, size);

	if(!file_.gcount()){ // read failure
		delete [] *output;
		*output = 0;
		return 0;
	}
	
	// write log
	wrtLog_<<"[get    ] off(KB): "<<setw(8)<<setfill('0')<<(address&0xfffffff)<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<
		" read(B): "<<setw(8)<<size<<endl;
	
	
	return size;

}

AddrType
Pool::del(AddrType address)
{
	idPool_.Release(address&0x0fffffff);

	// write log
	wrtLog_<<"[del    ] off(KB): "<<setw(8)<<setfill('0')<<(address&0xfffffff)<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<endl;

	return address;
}

AddrType
Pool::migrate(std::fstream &src_file, SizeType orig_size, 
	char const *data, SizeType size)
{
	// !! For eliminating seek operations
	// assume the pptr() of src_file is located 
	// in head of original data
	
	SizeType new_size = orig_size + size;

	if(!idPool_.avail()){
		return 0; // temp used
	}
	
	AddrType off = idPool_.Acquire();
	
	// clear() is required when previous read reach the file end
	file_.clear();
	file_.seekp(off * chunk_size_, ios::beg);
	
	
	// write 8 bytes size value ahead
	file_<<setw(8)<<setfill('0')<<new_size;
	
	SizeType toRead(orig_size);
	char buf[4096];
	while(toRead){
		if(toRead > 4096)
			src_file.read(buf, 4096);
		else
			src_file.read(buf, toRead);

		if(!src_file.gcount()){ // read failure
			return 0;
		}
		
		toRead -= src_file.gcount();
		file_.write(buf, src_file.gcount());
		if(!file_){ // write failure
			return 0;
		}
	}

	file_.write(data, size);
	
	if(!file_){ // write failure
		return 0;
	}
	
	// write log
	wrtLog_<<"[migrate] off(KB): "<<setw(8)<<setfill('0')<<off<<
		" tellp(B): "<<setw(12)<<file_.tellp()<<
		" migrate(B): "<<setw(8)<<orig_size<<
		" append(B): "<<size<<endl;

	return off;
}

// test main
#include <iostream>
#include <iomanip>
int main(int argc, char **argv)
{
	using namespace std;

	BehaviorDB bdb;
	AddrType addr1, addr2;

	char two_kb[2048] = "2k_data_tailed_by_null";
	char ten_kb[10240] = "10k_data_tailed_by_null";
	
	addr1 = bdb.put(two_kb, 2048);
	addr2 = bdb.append(addr1, ten_kb, 10240);

	return 0;	
}
