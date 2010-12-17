#ifndef _BEHAVIORDB_H
#define _BEHAVIORDB_H


/// @todo (Done) TODO: Configuration object for BehaviorDB
/// @todo TODO: Pure C wrapper
/// @todo TODO: iovec methods

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

/** Configuration object for BehaviorDB
 */
struct Config
{
	/** Indicate whether to write pool logs
	 */
	bool pool_log;

	/** The unit of chunk.
	 *  If this value is k, then the unit will be 2^k.
	 */
	SizeType chunk_unit;
	
	/** Setup default configuration
	 */
	Config():pool_log(true), chunk_unit(10){}
};

/// @todo TODO: Auto migration with liveness factor (improve put effectness)
/// BehaviorDB Interface
struct BehaviorDB
{
	
	BehaviorDB();

	/** Constructor that uses client's configuration
	 *  @param conf see Config
	 */
	BehaviorDB(Config const& conf);
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

	/** Enable/disable pool logs
	 *  @param do_log
	 *  @return Reference to *this
	 */
	BehaviorDB& 
	set_pool_log(bool do_log);
	
	/**
	 *  @brief Estimate max size corresponds to the address
	 *  @param address
	 *  @return Maximun size corresponeds to the address or 0 
	 *  when address overflowed.
	 */
	SizeType 
	estimate_max_size(AddrType address);

	/** @brief Estimate pool index according to size
	 *  @param size
	 *  @return Pool index or -1 when size to large.
	 */
	AddrType 
	estimate_pool_index(SizeType size);

	/** Error Number
	 *  @see ERRORNUMBER
	 */
	int error_num;
private:
	Config conf_;
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

/*! \mainpage Menual Page
 *  \section intro_sec Introduction
 *  
 *  BehaviorDB is a document oriented database. We provide high efficiency storing and retrieval
 *  particular for data with various size. Within the BehaviorDB, users can write data incrementaly
 *  without worry about extra cost of retrieving data.
 *
 *  This document is for internal development currently. You won't find any download link here.
 *  If anyone is interest in this project, please contact yangacer__at__gmail.
 *
 *  \section install_sec Installation
 *
 *  To be added.
 *
 *  \section transcation_sec Transcation
 *
 *  BehaviorDB manages addresses by a ID pool that write transcation logs to disk in every methods
 *  BehaviorDB provides. Notice when the ID pool fails to write log, system will be terminated
 *  immediately.
 *
 *  \section error_handle_sec Error Handling
 *
 *  BehaviorDB defines several error numbers. Any methods provided by BehaviorDB will store one of 
 *  those error number in BehaviorDB::error_num. Clients should check <strong>both</strong> return 
 *  value of methods and BehaviorDB::error_num.
 *
 *  <strong>Caution</strong><br/>
 *  Each method clears error number <strong>except the SYSTEM_ERROR</strong> automatically before
 *  processing request.
 *
 *  Hereby we give some checking examples:
 *
 *  1. BehaviorDB::put
 *  \code
 *  BehaviorDB bdb;
 *  char *data = "my data";
 *  AddrType rt = bdb.put(data, strlen(data));
 *  if( rt == -1 && bdb.error_num ){
 *	 if(bdb.error_num == SYSTEM_ERROR){
 *		fprintf(stderr, strerror(errno));
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
 *  2. BehaviorDB::append
 *  \code
 *  BehaviorDB bdb;
 *  AddrType r1, r2;
 *  // Eliminate error detection ...
 *  r1 = bdb.put("fisrt part", strlen("first part");
 *  r2 = bdb.append(r1, "second part", strlen("second part"));
 *  if(r2 == -1 && bdb.error_num){
 *	if(bdb.error_num == SYSTEM_ERROR){
 *		// ...
 *	}else{ 
 *		// ADDRESS_OVERFLOW or DATA_TOO_BIG
 *	}
 *  }
 *  \endcode
 *
 * 3. BehaviorDB::get
 * \code
 * // Assue addr is the address points to data we need
 * BehaviorDB bdb;
 * // Estimate maximun size of data by address
 * SizeType est_size = bdb.estimate_max_size(addr);
 * // BehaviorDB requies client pre-allocation 
 * char *buf = (char*)malloc(est_size * sizeof(char));
 * SizeType rt = bdb.get(buf, est_size, addr);
 * if(rt == -1 && bdb.error_num){
 *	if(bdb.error_num == SYSTEM_ERROR){
 *
 *	}else{
 *		// DATA_TOO_BIG
 *	}
 * }
 * \endcode
 * 4. BehaviorDB::del<br/>
 * This method does not set any error number but may result in system termination
 * if underlying transcation system crash.
 * see @ref transcation_sec for more information.
 */

