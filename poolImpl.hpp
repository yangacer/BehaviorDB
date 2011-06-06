#ifndef _POOL_HPP_
#define _POOL_HPP_

#include "config.hpp"
#include "addr_eval.hpp"
#include "fixedPool.hpp"
#include "chunk.h"
#include "idPool.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <utility>

#define MIGBUF_SIZ 2*1024*1024

namespace BDB
{
	
	struct viov;

	struct pool
	{
		struct config
		{
			unsigned int dirID;
			char const* work_dir;
			char const* trans_dir;
			char const* header_dir;
			addr_eval<AddrType> * addrEval;
			
			config()
			: dirID(0), 
			  work_dir(""), trans_dir(""), header_dir(""),
			  addrEval(0)
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
		
		AddrType
		write(viov *vv, size_t len);

		size_t
		read(char* buffer, size_t size, AddrType addr, size_t off=0, ChunkHeader const* header=0);

		AddrType
		merge_move(char const*data, size_t size, AddrType src_addr, size_t off,
			pool *dest_pool, ChunkHeader const* header=0);

		size_t
		erase(AddrType addr);

		size_t
		erase(AddrType addr, size_t off, size_t size);
	
		int
		head(ChunkHeader *header, AddrType addr) const;

		void
		on_error(int errcode, int line);
		
		std::pair<int, int>
		get_error();

		/* TODO: To be considered
		AddrType
		pine(AddrType addr);

		AddrType
		unpine(AddrType addr);
		

		std::pair<AddrType, size_t>
		tell2addr_off(off_t fpos) const;
		*/
		
	private:
		off_t
		seek(AddrType addr, size_t off =0);

		off_t
		addr_off2tell(AddrType addr, size_t off) const;
		
		/*
		void lock_acq();
		void lock_rel();
		*/

	private: // methods
		
		// TODO is ctor sufficient?
		// void create(config const& conf);

		pool(pool const& cp);
		pool& operator=(pool const& cp);

	private: // data member
		unsigned int dirID;
		std::string work_dir;
		std::string trans_dir;
		
		addr_eval<AddrType> const* addrEval;
		
		// pool file
		FILE *file_;
		char mig_buf_[MIGBUF_SIZ];
		
		// id file
		IDPool<AddrType> idPool_;

		// header
		fixed_pool<ChunkHeader, 8> headerPool_;
	public:	
		std::deque<std::pair<int,int> > err_;
	};
} // end of namespace BDB


#endif // end of pool.hpp guard
