#ifndef _IDPOOL_H
#define _IDPOOL_H

#include <deque>
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include "hash_helper.h"

/// @todo TODO: Replace IDPool data structure with bitset.
/// @todo TODO: Transaction file compression.

template<typename IDType>
class IDPool
{
public:
	IDPool()
	: 
	beg_(0), 
	cur_(0), 
	end_(std::numeric_limits<IDType>::max())
	{}
	
	IDPool(IDType begin_val, 
		IDType end_val = std::numeric_limits<IDType>::max() )
	: 
	beg_(begin_val), 
	cur_(begin_val), 
	end_(end_val)
	{}


	~IDPool()
	{
		fclose(file_); 
	}
	
	bool
	isAcquired(IDType const& id) const
	{ 
		if(id >= cur_ || id < beg_)
			return false;
		return h_.find(id) == h_.end();
	
	}
	
	IDType 
	next()
	{
		return cur_;	
	}
	
	unsigned long
	freeListSize() const
	{
		return h_.size();
	}

	IDType 
	Acquire()
	{

		if(!h_.empty()){
			IDType tmp(h_.begin()->second);
			if(0 > fprintf(file_, "+%u\n", tmp) && errno){
				fprintf(stderr, "idPool: %s\n", strerror(errno));
				exit(1);
			}
			h_.erase(h_.begin());
			return tmp;
		}
		if(0 > fprintf(file_, "+%u\n", cur_) && errno){
			fprintf(stderr, "idPool: %s\n", strerror(errno));
			exit(1);
		}
		return cur_++;
	}
	

	IDType
	Acquire_throw() throw(std::overflow_error)
	{
		if(!h_.empty()){
			IDType tmp(h_.begin()->second);
			if(0 > fprintf(file_, "+%u\n", tmp) && errno){
				fprintf(stderr, "idPool: %s\n", strerror(errno));
				exit(1);
			}
			h_.erase(h_.begin());
			return tmp;
		}

		if(cur_+ 1 == end_)
			throw std::overflow_error("IDPool: ID overflowed");

		if(0 > fprintf(file_, "+%u\n", cur_) && errno){
			fprintf(stderr, strerror(errno));
			exit(1);
		}
		return cur_++;
	
	}
	
	void 
	Release(IDType const &id)
	{
		if(id < beg_ && id >= cur_)
			return;
		
		if(0 > fprintf(file_, "-%u\n", id) && errno){
			fprintf(stderr,"%s\n", strerror(errno));
			exit(1);
		}

		if(cur_ == id + 1){
			--cur_;
			return;
		}
		h_[id] = id;
		return;
	}
	
	void
	Release_throw(IDType const &id) throw(std::out_of_range)
	{
		if(id < beg_ || id >= cur_)
			throw std::out_of_range("IDPool: Range error");
		
		if(0 > fprintf(file_, "-%u\n", id) && errno){
			fprintf(stderr, strerror(errno));
			exit(1);
		}
		if(cur_ == id + 1){
			--cur_;
			return;
		}
		h_[id] = id;
		return;

	}


	bool 
	avail() const
	{ return ( cur_ + 1 != end_ || !h_.empty() ); }


	void replay_transaction(char const* transaction_file)
	{
		FILE *tfile = fopen(transaction_file, "r+");

		if(0 == tfile){ // no transaction files for replaying
			//fprintf(stderr, "No transaction replay at %s\n", transaction_file);
			errno = 0;
			return;
		}
		

		char line[21] = {0};		
		IDType id;
		while(fgets(line, 20, tfile)){
			line[strlen(line)-1] = 0;
			id = strtoul(&line[1], 0, 10);
			if('+' == line[0]){
				if(!h_.empty())
					h_.erase(id);
				else
					cur_ = id+1;
			}else if('-' == line[0]){
				if(id == cur_ -1)
					cur_--;
				else
					h_[id] = id;
			}
		}
		fclose(tfile);
	}
	
	void init_transaction(char const* transaction_file) throw(std::runtime_error)
	{
		
		if(0 == (file_ = fopen(transaction_file,"a"))){
			fprintf(stderr, "Fail to open %s; system(%s)\n", transaction_file, strerror(errno));
			throw std::runtime_error("Fail to open transaction file");
		}
		
		if(0 != setvbuf(file_, (char*)0, _IONBF, 0)){
			fprintf(stderr, "Fail to set zero buffer on %s;system(%s)\n", transaction_file, strerror(errno));
			throw std::runtime_error("Fail to set zero buffer on transaction_file");
		}
		

	}

//private:

	IDPool(IDPool const &cp);
	IDPool& operator=(IDPool const &cp);

	IDType const beg_, end_;
	IDType cur_;
	STL::hash_map<IDType, IDType> h_;
	FILE*  file_;

};


#endif

