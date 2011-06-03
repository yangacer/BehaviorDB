#ifndef _POOL_HPP_
#define _POOL_HPP_

#include "config.hpp"
#include "addr_eval.hpp"
#include "chunk.h"
#include "idPool.h"
#include <string>
#include <cstdio>
#include <cstdlib>

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
			: dirID(0), work_dir(""), addrEval(0)
			{}
		};
		
		pool();
		pool(config const &conf);
		~pool();
		
		operator void const*() const;

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
		FILE *file_;
		char mig_buf_[MIGBUF_SIZ];

	};
} // end of namespace BDB


#endif // end of pool.hpp guard
