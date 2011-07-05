#include <algorithm>
#include "refHistory.h"

struct op_addr {
	char op;
	unsigned int address;

	op_addr(char op, unsigned int address)
		: op(op), address(address)
	{}
};

refHistory::refHistory()
: maxSize_(0), 
dq_(new ListType())
{}

refHistory::refHistory(unsigned int size)
: maxSize_(size),
dq_(new ListType())
{}

refHistory::~refHistory()
{
	delete dq_;	
}

void
refHistory::set_size(unsigned int size)
{
	while(size < maxSize_){
		dq_->pop_front();
		--maxSize_;	
	}
	maxSize_ = size;
}

void 
refHistory::add(char op, unsigned int address)
{
	if(size() >= maxSize_)
		dq_->pop_front();	
	
	dq_->push_back(op_addr(op, address));
}

void 
refHistory::update(unsigned int oldAddr, unsigned int newAddr)
{
	//std::replace(dq_->begin(), dq_->end(), oldAddr, newAddr);
	
	for(ListType::iterator iter = dq_->begin(); iter != dq_->end(); ++iter){
		if(iter->address == oldAddr)
			iter->address = newAddr;
	}
}

struct AddrEq {
	unsigned int address;
	
	AddrEq(unsigned address)
	:address(address)
	{}

	bool operator()(op_addr const &operand)
	{
		return address == operand.address;	
	}
};

void 
refHistory::remove(unsigned int address)
{
	dq_->erase(std::remove_if(dq_->begin(), dq_->end(), AddrEq(address)), dq_->end());
	
}

struct Count_by_Op {
	countResult r;
	unsigned int address;

	Count_by_Op(unsigned int address)
	: address(address)
	{
		for(int i=0;i<OPCNT;++i)
			r.count[i] = 0;
	}
	
	void operator()(op_addr const &item)
	{
		if(item.address == address)
			r.count[item.op]++;		
	}
};


countResult
refHistory::count(unsigned int address) const
{
	Count_by_Op cbo(address);
	cbo = for_each(dq_->begin(), dq_->end(), cbo);
	//for(ListType::iterator iter = dq_->begin(); iter != dq_->end();++iter)
	//	cbo(*iter);
	return cbo.r;
}

unsigned int
refHistory::size() const
{
	return dq_->size();
}
