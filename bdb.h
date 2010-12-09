#ifndef _BEHAVIORDB_H
#define _BEHAVIORDB_H

#include <iosfwd>

enum ERRORNUMBER
{
	ADDRESS_OVERFLOW = 1,
	SYSTEM_ERROR = 2,
	DATA_TOO_BIG = 4,
	ALLOC_FAILURE = 8
};

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

	int error_num;
private:
	// copy, assignment
	BehaviorDB(BehaviorDB const &cp);
	BehaviorDB& operator = (BehaviorDB const &cp);
	
	void clear_error();
	bool error_return();
	void log_access(char const* operation, AddrType address, SizeType size, char const* desc = 0);

	Pool* pools_;
	std::ofstream *accLog_, *errLog_;
};

#endif // header ends

