#ifndef _POOL_HPP_
#define _POOL_HPP_

#include "config.hpp"
#include "addr_eval.hpp"
#include "bdb.h"
#include "chunk.h"
#include "idPool.h"
#include <fstream>
#include <cstdio>
#include <string>

#define MIGBUF_SIZ 2*1024*1024

namespace BDB
{
	struct pool
	{
		struct config
		{
			unsigned char dirID;
			char const* work_dir;
			addr_eval<AddrType> * addrEval;
			config()
			: dirID(0), work_dir("")
			{}
		};

		pool(config const &conf);
		~pool();
		
		AddrType
		write(char const* data, size_t size);
		
		// off=-1 represent an append write
		AddrType
		write(char const* data, size_t size, AddrType addr, size_t off=-1, ChunkHeader const* header=0);
		
		size_t
		read(char* buffer, size_t size, AddrType addr, size_t off=0, ChunkHeader const* header=0);

		AddrType
		merge_move(AddrType src_addr, size_t off, char const*data, size_t size,
			pool *dest_pool, ChunkHeader const* header=0);

		size_t
		erase(AddrType addr);

		size_t
		erase(AddrType addr, size_t off, size_t size);
		
		ChunkHeader
		head(AddrType addr, size_t off = -1);
		
		// TODO make sure windows can provide/simulate off_t
		off_t
		seek(AddrType addr, size_t off =0);

		/* TODO: To be considered
		AddrType
		pine(AddrType addr);

		AddrType
		unpine(AddrType addr);
		
		std::pair<AddrType, size_t>
		tell2addr_off(std::streampos fpos) const;

		std::streampos
		addr_off2tell(AddrType addr, size_t off) const;
		
		void lock_acq();
		void lock_rel();
		*/

	private: // methods
		
		// TODO is ctor sufficient?
		// void create(config const& conf);

		pool(pool const& cp);
		pool& operator=(pool const& cp);

	private: // data member
		unsigned char dirID;
		std::string work_dir;
		addr_eval<AddrType> * addrEval;
		IDPool<AddrType> idPool_;
		std::fstream file_;
		char mig_buf_[MIGBUF_SIZ];

	};
} // end of namespace BDB

//! \brief Pool - A Chunk Manager
struct Pool
{
	friend struct BehaviorDB;
	friend struct AddrIterator;

	Pool();
	~Pool();

	/** Create chunk file
	 *  @param chunk_size
	 *  @param conf
	 *  @remark Error Number: none.
	 *  @remark Any failure happend in this method causes
	 *  system termination.
	 */
	void 
	create_chunk_file(SizeType chunk_size, Config const &conf);
	
	/** Enable/disable logging of pool
	 *  @param do_log
	 *  @remark BehaviorDB enable pool log by default.
	 */
	void
	log(bool do_log);

	/** Get chunk size
	 *  @return Chunk size of this pool.
	 */
	SizeType 
	chunk_size() const;

	/** Put data to some chunk
	 *  @param data Data to be put into this pool.
	 *  @param size Size of the data.
	 *  @return Address for accesing data just put.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW.
	 */
	AddrType 
	put(char const* data, SizeType size);
	
	/** Overwrite a chunk
	 *  @param address
	 *  @param data
	 *  @param size
	 *  @return Address
	 *  @remark Error Number: SYSTEM_ERROR, NON_EXIST
	 */
	AddrType
	overwrite(AddrType address, char const *data, SizeType size);

	/** Append data to a chunk
	 *  @param address Indicate which chunk to be appended.
	 *  @param data Data to be appended.
	 *  @param size Size of the data.
	 *  @param next_pool_idx Next pool index estimated by BehaviorDB.
	 *  @param next_pool Pointer to next pool refered by next_pool_idx.
	 *  @return Address for accessing the chunk that stores concatenated data.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW, DATA_TOO_BIG.
	 */
	AddrType 
	append(AddrType address, char const* data, SizeType size, 
		AddrType next_pool_idx, Pool* next_pool);
	
	/** Get chunk
	 *  @param output Output buffer for placing retrieved data.
	 *  @param size Size of output buffer
	 *  @param address Indicate which chunk to be retrieved.
	 *  @return Size of the output buffer.
	 *  @remark Error Number: SYSTEM_ERROR, DATA_TOO_BIG.
	 *  @remark In order to enhance security of library. 
	 *  Client has to be responsible for ensuring the size of output buffer
	 *  being large enough.
	 */
	SizeType 
	get(char *output, SizeType const size, AddrType address);
	
	SizeType
	get(char *output, SizeType const size, AddrType address, StreamState *stream);
	
	/** Delete chunk
	 *  @param address Address of chunk to be deleted.
	 *  @return Address of chunk just deleted.
	 *  @remark Error Number: None.
	 */
	AddrType
	del(AddrType address);

	/** Error number.
	 *  Not zero when any applicaton or system error happened.
	 */
	int error_num;

protected:
	
	/** Seek to chunk header
	 *  @param address
	 *  @remark Error Number: SYSTEM_ERROR
	 */
	void 
	seekToHeader(AddrType address);


	/** Move data from one chunk to another pool
	 *  @param src_file File stream of another pool which had seeked to source data.
	 *  @param orig_size Size of original data.
	 *  @param data Data to be appended to the original data.
	 *  @param size Size of appended data.
	 *  @return Address of the chunk that stores concatenated data.
	 *  @remark Error Number: SYSTEM_ERROR, ADDRESS_OVERFLOW
	 */
	AddrType
	migrate(std::fstream &src_file, ChunkHeader ch, 
		char const *data, SizeType size); 

	void
	write_log(char const *operation, 
		AddrType const* address,
		std::streampos tell,
		SizeType size = 0,
		char const *desc = 0,
		int src_line = 0);

	void clear_error();

private:
	Pool(Pool const &cp);
	Pool& operator=(Pool const &cp);
	
	Config conf_;
	SizeType chunk_size_;
	bool doLog_;
	std::fstream file_;
	IDPool<AddrType> idPool_;
	std::ofstream wrtLog_;
	char file_buf_[1024*1024];
	static char migbuf_[MIGBUF_SIZ];
	bool onStreaming_;

	int lock_;
	static refHistory rh_;
	static BehaviorDB::MigPredictorCB pred_;
};

#endif // end of pool.hpp guard
