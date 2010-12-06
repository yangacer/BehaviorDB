#ifndef _IDPOOL_H
#define _IDPOOL_H

#include <deque>
#include <stdexcept>
#include <limits>

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
			return tmp;
		}
		return cur_++;
	}
	
	IDType
	Acquire_throw() throw(std::overflow_error)
	{
		if(!q_.empty()){
			IDType tmp(q_.back());
			q_.pop_back();
			return tmp;
		}

		if(cur_+ 1 == end_)
			throw std::overflow_error("IDPool: ID overflowed");

		return cur_++;
	
	}
	
	void 
	Release(IDType const &id)
	{
		if(id < beg_ && id >= cur_)
			return;
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
		if(cur_ == id + 1){
			--cur_;
			return;
		}
		q_.push_back(id);
		return;

	}


	bool 
	avail() const
	{ return (cur_ + 1 != end_ || q_.empty()); }

private:
	IDPool(IDPool const &cp);
	IDPool& operator=(IDPool const &cp);

	IDType const beg_, end_;
	IDType cur_;
	std::deque<IDType> q_;
};

#endif

