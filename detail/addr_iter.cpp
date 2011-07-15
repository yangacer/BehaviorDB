#include <stdexcept>
#include "addr_iter.hpp"
#include "bdbImpl.hpp"
#include "idPool.hpp"
#include "addr_eval.hpp"

namespace BDB {

	AddrIterator::AddrIterator()
	: cur_(0), bdb_(0)
	{}

	AddrIterator::AddrIterator(AddrIterator const &cp)
	: cur_(cp.cur_), bdb_(cp.bdb_)
	{}
	
	AddrIterator&
	AddrIterator::operator=(AddrIterator const &cp)
	{
		if(*this == cp) return *this;	
		cur_ = cp.cur_;
		bdb_ = cp.bdb_;
		return *this;
	}

	AddrIterator&
	AddrIterator::operator++()
	{

		if(0 == bdb_) 
			throw std::logic_error("AddrIterator: No BehaviorDB instance, "
				"you should use iter = bdb.begin().");

		cur_ = bdb_->global_id_->next_used(cur_+1);
		return *this;
	}
	
	AddrType
	AddrIterator::operator *() const
	{ 
		if(0 == bdb_) 
			throw std::logic_error("AddrIterator: No BehaviorDB instance, "
				"you should use iter = bdb.begin().");

		if( !bdb_->global_id_->isAcquired(cur_) )
			throw std::out_of_range("AddrIterator: Iterator is invalid");

		return cur_;
	}

	bool
	AddrIterator::operator ==(AddrIterator const &rhs) const
	{ 
		if(0 == bdb_) 
			throw std::logic_error("AddrIterator: No BehaviorDB instance, "
				"you should use iter = bdb.begin().");

		return cur_ == rhs.cur_ && bdb_ == rhs.bdb_; 
	}

	AddrIterator::AddrIterator(BDBImpl const& bdb, AddrType cur)
	: cur_(cur), bdb_(&bdb)
	{}

} // end of namespace BDB
