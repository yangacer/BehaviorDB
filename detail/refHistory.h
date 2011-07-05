#ifndef _REFHISTORY_H
#define _REFHISTORY_H

#include <deque>

#define OPCNT 3

enum OPERATION {
	PUT=0, APPEND, GET
};

struct countResult
{
	unsigned int count[OPCNT];
};

struct op_addr;

struct refHistory
{
	refHistory();
	refHistory(unsigned int size);
	~refHistory();

	void 
	set_size(unsigned int size);

	void 
	add(char op, unsigned int address);
	
	void 
	update(unsigned int oldAddr, unsigned int newAddr);
	
	void 
	remove(unsigned int address);
	
	countResult
	count(unsigned int address) const;

	unsigned int
	size() const;

private:

	refHistory(refHistory const &cp);
	refHistory& operator=(refHistory const &cp);
	
	unsigned int maxSize_;
	
	typedef std::deque<op_addr> ListType;
	ListType *dq_;
}; 

#endif
