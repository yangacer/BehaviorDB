#ifndef _BDBIMPL_HPP
#define _BDBIMPL_HPP

#include <cstdio>
#include <string>
#include "common.hpp"
#include "addr_eval.hpp"
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"
#include "boost/pool/object_pool.hpp"
#include "boost/noncopyable.hpp"

namespace BDB {
	
	class IDValPool;
	struct pool;
	struct AddrIterator;	

	struct BDBImpl 
  : boost::noncopyable
	{
		friend struct bdbStater;
    friend struct AddrIterator;

    BDBImpl(Config const & conf);
    ~BDBImpl();
		
		//operator void const*() const;
		
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
    
    // ------------ Transparent Interfaces --------------
    // nt stands for no internal/external address translation is performed

		AddrType
		nt_put(char const *data, size_t size);

		AddrType
		nt_put(char const *data, size_t size, AddrType addr, size_t off=npos);
			
		AddrType
		nt_put(std::string const& data)
		{ return put(data.data(), data.size()); }
		
		AddrType
		nt_put(std::string const& data, AddrType addr, size_t off=npos)
		{ return put(data.data(), data.size(), addr, off); }
		
		AddrType
		nt_update(char const *data, size_t size, AddrType addr);
		
		AddrType
		nt_update(std::string const& data, AddrType addr)
		{ return update(data.data(), data.size(), addr); }

		size_t
		nt_get(char *output, size_t size, AddrType addr, size_t off=0);
		
		size_t
		nt_get(std::string *output, size_t max, AddrType addr, size_t off=0);

		size_t
		nt_del(AddrType addr);

		size_t
		nt_del(AddrType addr, size_t off, size_t size);
    
    // TODO nt_ for BDB::i/ostream function

    // ------------ Transparent Interfaces End ----------

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
		
		AddrType
		stream_finish(stream_state const* state);
		
		size_t
		stream_pause(stream_state const* state);

		stream_state const*
		stream_resume(size_t encrypt_handle);
		
		void
		stream_expire(size_t encrypt_handle);

		void
		stream_abort(stream_state const* state);
		
		bool
		stream_error(stream_state const* state);

		AddrIterator
		begin() const;

		AddrIterator
		end() const;
		
		void stat(Stat* s) const;
		
		bool full() const;

	protected:
    
    // write data to pool
    // Return -1 if fail, otherwise returns internal address
		AddrType
    write_pool(char const*data, size_t size);
    
    // handle error triggered in pool(s)
		void
		error(unsigned int dir);

		// handle error triggered in BDBImpl
		void
		error(int errcode, int line);
	  

	private:
		typedef boost::unordered_map<AddrType, unsigned int> AddrCntCont;
		typedef boost::unordered_set<size_t> EncStreamCont;
		
		addr_eval<AddrType> addrEval;
		pool* pools_;
		FILE* err_log_;
		char err_log_buf_[256];
		
		FILE* acc_log_;
		char acc_log_buf_[4096];
		IDValPool *global_id_;
		
		AddrCntCont in_reading_;
		// TODO two containers as follows are not recoverable
		EncStreamCont enc_stream_state_;
		boost::object_pool<stream_state> stream_state_pool_;
	};

} // end of namespace BDB

#endif // end of header
