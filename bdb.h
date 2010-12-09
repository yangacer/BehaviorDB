#ifndef _BEHAVIORDB_H
#define _BEHAVIORDB_H

#include <iosfwd>

/** @file bdb.h
 *  @brief BehaviorDB header file
 */

//! Error numbers
enum ERRORNUMBER
{
	/// Pool can not address more chunks
	ADDRESS_OVERFLOW = 1,

	/// I/O operation failure
	SYSTEM_ERROR = 2,

	/// Data is too big to be handle by BehaviorDB 
	DATA_TOO_BIG = 4,

	/// Deprecated
	ALLOC_FAILURE = 8
};

//! Define address type.
typedef unsigned int AddrType;

//! Define address type. 
typedef unsigned int SizeType;

// Forward decl
struct Pool;

/// BehaviorDB Interface
struct BehaviorDB
{
	
	BehaviorDB();
	~BehaviorDB();
	
	/** Put data into a chunk
	 *  @param data
	 *  @param size
	 *  @return Address of the chunk that stores the data or -1 when error happened.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG
	 */
	AddrType 
	put(char const* data, SizeType size);
	
	/** Append data to a chunk
	 *  @param address
	 *  @param data
	 *  @param size
	 *  @return Address of a chunk that stores concatenated data or -1 when errror happened.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG.
	 */
	AddrType 
	append(AddrType address, char const* data, SizeType size);
	
	/** Get data from a chunk
	 *  @param output Output buffer for placing data
	 *  @param size Size of output buffer
	 *  @param address Address of a chunk
	 *  @return Size of data that stored in the chunk or -1 when error happened.
	 *  @remark Error Number: SYSTEM_ERROR, DATA_TOO_BIG
	 *  @remark In order to enhance security of library. 
	 *  Client has to be responsible for ensuring the size of output buffer
	 *  being large enough.
	 *  @see estimate_max_size
	 */
	SizeType 
	get(char *output, SizeType const size, AddrType address);

	/** Delete a chunk
	 *  @param address
	 *  @return Address of just deleted chunk
	 *  @remark Error Number: None.
	 */
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

/**
 *  @brief Estimate max size corresponds to the address
 *  @param address
 *  @return Maximun size corresponeds to the address or 0 
 *  when address overflowed.
 */
inline SizeType 
estimate_max_size(AddrType address);

/** @brief Estimate pool index according to size
 *  @param size
 *  @return Pool index or -1 when size to large.
 */
inline AddrType 
estimate_pool_index(SizeType size);

#endif // header ends

/*! \mainpage Menual Page
 *  \section intro_sec Introduction
 *  
 *  To be added.
 *
 *  \section install_sec Installation
 *
 *  To be added.
 *
 *  \section error_handle_sec Error Handling
 *
 *  BehaviorDB defines several error numbers. Any methods provided by BehaviorDB will store one of 
 *  those error number in BehaviorDB::error_num. Clients should check <strong>both</strong> return 
 *  value of methods and BehaviorDB::error_num.
 *
 *  Hereby we give some checking examples:
 *
 *  1. BehaviorDB::put
 *  \code
 *  BehaviorDB bdb;
 *  char *data = "my data";
 *  SizeType rt = bdb.put(data, strlen(data));
 *  if( rt == -1 && bdb.error_num ){
 *	fprintf(stderr, strerror(errno));
 *	if(bdb.error_num == SYSTEM_ERROR){
 *		exit(1);	// Exit since system error can not be recovered
 *				// without shutdown BehaviorDB
 *	}else{  // true == (bdb.error_num & (ADDRESS_OVERFLOW | DATA_TOO_BIG)))
 *		// split data into several parts and retry
 *	}	
 *  }else{
 *	// success put
 *  }
 *  \endcode
 *  
 *
 */
