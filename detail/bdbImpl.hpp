#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP

#include <cstdio>
#include <string>
#include "common.hpp"
#include "addr_eval.hpp"


namespace BDB {
	
	class IDValPool;
	struct pool;
	struct AddrIterator;	

	struct BDBImpl 
	{
		friend struct bdbStater;
		friend struct AddrIterator;

	        BDBImpl(Config const & conf);
		~BDBImpl();
		
		operator void const*() const;
		
		void
		init_(Config const & conf);


		/** @brief Put data
		 *  @param data
		 *  @param size
		 *  @return Global address
		 */
		AddrType
		put(char const *data, size_t size);

		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=npos);
			
		AddrType
		put(std::string const& data)
		{ return put(data.data(), data.size()); }
		
		AddrType
		put(std::string const& data, AddrType addr, size_t off=npos)
		{ return put(data.data(), data.size(), addr, off); }
		
		AddrType
		preserve(size_t preserve_size, char const *data=0, size_t size=0);

		AddrType
		update(char const *data, size_t size, AddrType addr);
		
		AddrType
		update(std::string const& data, AddrType addr)
		{ return update(data.data(), data.size(), addr); }

		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
		size_t
		get(std::string *output, size_t max, AddrType addr, size_t off=0);

		size_t
		del(AddrType addr);

		size_t
		del(AddrType addr, size_t off, size_t size);

		// streaming interface
		stream_state const*
		ostream(size_t stream_size);

		stream_state const*
		ostream(size_t stream_size, AddrType addr, size_t off=npos);

		stream_state const*
		istream(size_t stream_size, AddrType addr, size_t off=0);
		
		stream_state const*
		stream_write(stream_state const* state, char const* data, size_t size);
		
		stream_state const*
		stream_read(stream_state const* state, char* output, size_t size);

		void
		stream_abort(stream_state const* state);

		AddrIterator
		begin() const;

		AddrIterator
		end() const;
		
		void stat(Stat* s) const;

	private: // disable interfaces
		BDBImpl(BDBImpl const& cp);
		BDBImpl& operator=(BDBImpl const &cp);

	protected:
		// handle error triggered in pool(s)
		void
		error(unsigned int dir);

		// handle error triggered in BDBImpl
		void
		error(int errcode, int line);
	
	private:
		typedef addr_eval<AddrType> addrEval;

		pool* pools_;
		FILE* err_log_;
		char err_log_buf_[256];
		
		FILE* acc_log_;
		char acc_log_buf_[256];
		IDValPool *global_id_;

	};

} // end of namespace BDB

#endif // end of header
