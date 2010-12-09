#ifndef _IDPOOL_H
#define _IDPOOL_H

#include <deque>
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

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
	{ if(file_) fclose(file_); }
	
	bool
	isAcquired(IDType const& id) const
	{ 
		if(id >= cur_ || id < beg_)
			return false;
		// perform search in retired list
		if( q_.rend() != find(q_.rbegin(), q_.rend(), id) )
			return false;
		return true;
	
	}

	IDType 
	Acquire()
	{
		if(!q_.empty()){
			IDType tmp(q_.back());
			q_.pop_back();
			if(0 > fprintf(file_, "+%u\n", tmp) && errno){
				fprintf(stderr, "idPool: %s\n", strerror(errno));
				exit(1);
			}
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
		if(!q_.empty()){
			IDType tmp(q_.back());
			q_.pop_back();
			if(0 > fprintf(file_, "+%u\n", tmp) && errno){
				fprintf(stderr, "idPool: %s\n", strerror(errno));
				exit(1);
			}
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
		q_.push_back(id);
		return;
	}
	
	void
	Release_throw(IDType const &id) throw(std::out_of_range)
	{
		if(id < beg_ || id >= cur_)
			throw std::out_of_range("IDPool: Range error");
		;
		if(0 > fprintf(file_, "-%u\n", id) && errno){
			fprintf(stderr, strerror(errno));
			exit(1);
		}
		if(cur_ == id + 1){
			--cur_;
			return;
		}
		q_.push_back(id);
		return;

	}


	bool 
	avail() const
	{ return ( cur_ + 1 != end_ || !q_.empty() ); }


	void replay_transcation(char const* transcation_file)
	{
		FILE *tfile = fopen(transcation_file, "r+");
		
		if(0 == tfile && EINVAL == errno){ // no transcation files for replaying
			errno = 0;
			return;
		}

		char line[21] = {0};		
		IDType id;
		while(fgets(line, 20, tfile)){
			line[strlen(line)-1] = 0;
			id = strtoul(&line[1], 0, 10);
			if('+' == line[0]){
				if(!q_.empty())
					q_.pop_back();
				else
					cur_ = id+1;
			}else if('-' == line[0]){
				if(id == cur_ -1)
					cur_--;
				else
					q_.push_back(id);
			}
		}

		fclose(tfile);
	}
	
	void init_transcation(char const* transcation_file) throw(std::runtime_error)
	{
		file_ = fopen(transcation_file, "a");
		if(0 == file_){
			fprintf(stderr, "Fail to open %s;system(%s)\n", transcation_file, strerror(errno));
			throw std::runtime_error("Fail to open transcation file");
		}

		if(0 != setvbuf(file_, (char*)0, _IONBF, 0)){
			fprintf(stderr, "Fail to set zero buffer on %s;system(%s)\n", transcation_file, strerror(errno));
			throw std::runtime_error("Fail to set zero buffer on transcation_file");
		}

	}

private:

	IDPool(IDPool const &cp);
	IDPool& operator=(IDPool const &cp);

	IDType const beg_, end_;
	IDType cur_;
	std::deque<IDType> q_;
	FILE* file_;

};


#endif

