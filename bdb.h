#ifndef _BEHAVIORDB_H
#define _BEHAVIORDB_H


/// @todo (Done) TODO: Configuration object for BehaviorDB
/// @todo TODO: Pure C wrapper
/// @todo (Done) TODO: iovec methods
/// @todo TODO: Windows Version
/// @todo TODO: Early migration (history estimation)
/// @todo (Done) TODO: Allow put small data to large chunk
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
	DATA_TOO_BIG = 3,

	/// Pool is on streaming
	POOL_LOCKED = 4,

	NON_EXIST = 5
};

//! Define address type.
typedef unsigned int AddrType;

//! Define size type. 
typedef unsigned int SizeType;


/** @brief Configuration object for BehaviorDB
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
	
	/** Threshold of early migration
	 */
	SizeType migrate_threshold;
	
	/** Working Directory for BehaviorDB
	 */
	char const * working_dir;

	/** Setup default configuration
	 */
	Config()
	:pool_log(true), chunk_unit(10), migrate_threshold(0x7f), working_dir(".")
	{}
};

/** @brief Output vector for BehaviorDB::append
 */
struct WriteVector
{
	char const* buffer;
	SizeType size;
	AddrType *address;
};

// Forward decls
struct Pool;
struct AddrIterator;
struct BehaviorDB;

/** @brief For recording input stream state
 */
struct StreamState
{
	friend struct BehaviorDB;
	friend struct Pool;

	StreamState():left_(0), pool_(0){}
private:
	SizeType left_;
	Pool* pool_;
};

/// BehaviorDB Interface
struct BehaviorDB
{
	friend struct AddrIterator;
	BehaviorDB();

	/** Constructor that uses client's configuration
	 *  @param conf See Config
	 */
	BehaviorDB(Config const& conf);
	~BehaviorDB();
	
	/** Put data into a chunk
	 *  @param data
	 *  @param size
	 *  @return Address of the chunk that stores the data or -1 when error happened.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG, POOL_LOCKED.
	 */
	AddrType 
	put(char const* data, SizeType size);
	
	/** Overwrite a chunk
	 *  @param address
	 *  @param data
	 *  @param size
	 *  @return Address of the overwritten chunk
	 *  @remark Error Number: SYSTEM_ERROR, NON_EXIST, DATA_TOO_BIG, POOL_LOCKED.
	 */
	AddrType
	overwrite(AddrType address, char const* data, SizeType size);

	/** Append data contained by WriteVector(s)
	 *  @param wv
	 *  @param wv_size
	 *  @return The first failure WriteVector. i.e. wv_size for non failure.
	 */
	int
	append(WriteVector *wv, int wv_size);

	/** Append data to a chunk
	 *  @param address
	 *  @param data
	 *  @param size
	 *  @return Address of a chunk that stores concatenated data or -1 when errror happened.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG, POOL_LOCKED.
	 *  @see @link append.cpp @endlink
	 */
	AddrType 
	append(AddrType address, char const* data, SizeType size);
	

	/** Get data from a chunk
	 *  @param output Output buffer for placing data
	 *  @param size Size of output buffer
	 *  @param address Address of a chunk
	 *  @return Size of data that stored in the chunk or -1 when error happened.
	 *  @remark Error Number: SYSTEM_ERROR, DATA_TOO_BIG, POOL_LOCKED.
	 *  @remark In order to enhance security of library. 
	 *  Client has to be responsible for ensuring the size of output buffer
	 *  being large enough.
	 *  @see estimate_max_size
	 */
	SizeType 
	get(char *output, SizeType const size, AddrType address);

	/** Get data from a chunk incrementaly
	 *  @param output
	 *  @param size
	 *  @param address
	 *  @param stream Stream state
	 *  @return Size of data that stored in the chunk or -1 when error happened.
	 *  @remark  Error Number: SYSTEM_ERROR, DATA_TOO_BIG, POOL_LOCKED.
	 *  @see @link getstream.cpp @endlink
	 */
	SizeType
	get(char *output, SizeType const size, AddrType address, StreamState* stream);
	
	/** Stop incremental get explicitly
	 *  @param stream
	 *  @see @link getstream.cpp @endlink
	 */
	void
	stop_get(StreamState* stream);

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
	 *  @remark The only configuration can be changed at runtime is pool logging.
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
	
	AddrIterator
	begin();

	AddrIterator
	end();

	/** Error Number
	 *  @see ERRORNUMBER
	 */
	int error_num;
private:
	void init_();
	
	Config conf_;
	// copy, assignment
	BehaviorDB(BehaviorDB const &cp);
	BehaviorDB& operator = (BehaviorDB const &cp);
	
	void clear_error();
	bool error_return();
	void log_access(char const* operation, AddrType address, SizeType size, char const* desc = 0);

	Pool* pools_;
	std::ofstream *accLog_, *errLog_;
	char accBuf_[1000000];

	int lock_;
};

/** @brief Iterator for iterating used addresses of BehaviorDB
 */
struct AddrIterator
{
	friend struct BehaviorDB;

	AddrIterator();
	AddrIterator(AddrIterator const& cp);

	AddrIterator& 
	operator=(AddrIterator const& cp);

	/** Advance iterator to next used address
	 */
	AddrIterator &
	operator++();
	
	/** Get the address on iterating
	 */
	AddrType 
	operator*();
	
	bool 
	operator==(AddrIterator const& rhs) const;
	
	bool 
	operator!=(AddrIterator const& rhs) const
	{ return !(*this == rhs); }

protected:
	AddrIterator(BehaviorDB &bdb, AddrType cur);

private:
	AddrType cur_;
	BehaviorDB *bdb_;
};


#endif // header ends

/*! \mainpage Menual Page
 *  \section intro_sec Introduction
 *  
 *  BehaviorDB is a document oriented database. We provide high efficiency storing and retrieval
 *  particular for data with various size. This database is designed to be a local storage management
 *  system that concentrate on optmizing disk I/O cost.
 *
 *  This document is for internal development currently. You won't find any download link here.
 *  If anyone is interest in this project, please contact yangacer__at__gmail.
 *
 *  \section install_sec Installation
 *  
 *  Following instructions require CMake 2.8+ tool. See the official site, http://www.cmake.org 
 *  for more information.
 *  BehaviorDB supports UNIX-like platform currently. To make it compatible with Windows system, 
 *  some modification related to path and path delimeter are required.
 *
 *  \code
 *  # UNIX
 *  cd BehaviorDB
 *  mkdir -p build
 *  cd build
 *  cmake .. -DCMAKE_INSTALL_PREFIX=/path -DINSTALL_TEST=ON
 *  # Or setup via ccmake
 *  make
 *  make install
 *  \endcode
 *  
 *  Option Specification: [default value]<p/>
 *
 *  1. CMAKE_INSTALL_PREFIX [/usr/local]<br/>
 *  The directory you want to install BehaviorDB header and library.<p/>
 *
 *  2. INSTALL_TEST [ON]<br/> 
 *  Install test programs and clean.sh. The clean.sh removes all pool file, logs, and transaction logs.<p/>
 *
 *  \section scale_sec Scalability
 *  
 *  BehaviorDB uses 16 pools, each of them can store 2^28 or 268 million chunks at most. Chunk size depends
 *  on client's configuration which ranged between 4bytes and 64GB. See @ref config_sec for more information.<p/>
 *
 *  1. Maximun Size Configuration<br/>
 *  Minimal Chunk Size = 2^21 or 2MB.<br/>
 *  Maximal Chunk Size = 2^36 or 64GB.<br/>
 *  Total Capacity = 512TB * (64K-1) ~ 32,767PB<p/>
 *
 *  2. Minimun Size Configuration<br/>
 *  Minimal Chunk Size = 2^4 or 16byte.<br/>
 *  Maximal Chunk Size = 2^19 or 512KB.<br/>
 *  Total Capacity = 4GB * (64K -1) ~ 255TB.<p/>
 *	
 *  Notice the actual total capacity is restricted by underlying filesystem.
 *
 *  \section transaction_sec Transaction
 *
 *  BehaviorDB manages addresses by a ID pool that write transaction logs to disk in every methods
 *  BehaviorDB provides. Notice when the ID pool fails to write log, system will be terminated
 *  immediately.
 *
 *  \section config_sec Configuration
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
 * // Assume addr is the address points to data we need
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
 * if underlying transaction system crash.
 * see @ref transaction_sec for more information.
 */

/** \example append.cpp
 * 
 * This simple test puts 128 bytes data into BehaviorDB and keeps append 2k bytes data to the same chunk so 
 * that make it migrate to a larger pool until no larger pool available.
 * After such process, there will be only one migErr error logged in 8000.pool.log which is correct result.
 *
 * This test also outputs number of access to each pool.
 */

/** \example bench.cpp
 *  
 *  Benchmark for BehaviorDB and filesystem. This program also utilize WriteVector for imporoving performance.
 */

/**
 *  \example largeput.cpp
 */

/**
 *  \example getstream.cpp
 */
