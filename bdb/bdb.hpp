#ifndef _BDB_HPP
#define _BDB_HPP

#include <string>
#include "export.hpp"
#include "common.hpp"

namespace BDB {
<<<<<<< HEAD
	
	struct BDBImpl;
	struct AddrIterator;
	
    /** @brief Core class
     */
	struct BDB_EXPORT BehaviorDB
	{
		/** @brief Constructor
		 *  @param conf
		 *  @throw std::invalid_argument 
		 *  @throw std::bad_alloc 
		 *  @throw std::runtime_error
		 *  @throw std::length_error
		 *  @details Call conf.validate() internally to verify configuration.
		 */
        BehaviorDB(Config const & conf);

		~BehaviorDB();
		
		/** @brief Put data
		 *  @param data
		 *  @param size
		 *  @return Address of the stored data or -1 for failure.
		 */
		AddrType
		put(char const *data, size_t size);
		
		/** @brief Put data to a specific address
		 *  @param data
		 *  @param size
		 *  @param addr
		 *  @param off
		 *  @return Address of the stored data
		 *  @details If the off parameter is BDB::npos or is not given, this 
         *  method acts as an append operation.
         *  @return Address on success. -1 for failure.
		 */
		AddrType
		put(char const *data, size_t size, AddrType addr, size_t off=npos);
		
        /** @brief std::string version put method 
         */
		AddrType
		put(std::string const& data);
		
        /** @brief std::string version put-to-address method 
         */
		AddrType
		put(std::string const& data, AddrType addr, size_t off=npos);
		
        /** @brief Replace specific address with new data
         *  @param data New data.
         *  @param size Size of new data.
         *  @param addr Address.
         *  @return Address of the new data or -1 that indicates failure.
         */
		AddrType
		update(char const* data, size_t size, AddrType addr);
		
        /** @brief std::string version update
         */
		AddrType
		update(std::string const& data, AddrType addr);

        /** @brief Read data from specified address and offset to a output
         *  buffer.
         *  @param output Ouput buffer.
         *  @param size Size of output buffer.
         *  @param addr Address.
         *  @param off Offset.
         *  @return Size of data read successly.
         */
		size_t
		get(char *output, size_t size, AddrType addr, size_t off=0);
		
        /** @brief std::string version get method
         */
		size_t
		get(std::string *output, size_t max, AddrType addr, size_t off=0);

        /** @brief Delete specified address.
         *  @return 0 for success. -1 for failure.
         */
		size_t
		del(AddrType addr);

        /** @brief Delete a segment of an address.
         *  @return Size after deleting or -1 for failure.
         */
		size_t
		del(AddrType addr, size_t off, size_t size);
	
        /** @brief Create output stream handle, stream_state, for 
         *  asynchronous write.
         *  @param stream_size Future size this handle will be written.
         *  @return stream_state or NULL if any error occured.
         */
		stream_state const*
		ostream(size_t stream_size);
        
        /** @brief Create output stream handle, stream_state, for 
         *  asynchronous write from an existed address. Offset of a
         *  chunk refered by the address can be assigned optionally.
         *  Default offset is the end of a chunk refered by the address.
         *  @return stream_state or NULL if any error occured.
         */
		stream_state const*
		ostream(size_t stream_size, AddrType addr, size_t off=npos);
		
        
        /** @brief Create output stream handle for asynchronous read.
         *  @param stream_size Size to be read.
         *  @param addr
         *  @param off
         *  @return stream_state or NULL if any error occured.
         */
		stream_state const*
		istream(size_t stream_size, AddrType addr, size_t off=0);
        
        /** @brief Write data to a stream_state created by ostream method.
         *  @return stream_state or NULL if any error occured.
         */
		stream_state const*
		stream_write(stream_state const* state, char const* data, size_t size);
		
        /** @brief Read data from a stream_state created by istream method.
         *  @return stream_state or NULL if any error occured.
         */
		stream_state const*
		stream_read(stream_state const* state, char* output, size_t size);
        
        /** @brief Finish asynchronous read/write. 
         *  @return Address of the operated chunk.
         */
		AddrType
		stream_finish(stream_state const* state);
		
        /** @brief Pause an asynchronous read/write.
         *  @return Encrypted stream state.
         *  @remark The encryption is required for
         *  preventing directly delete/free to the 
         *  stream_state.
         */
		size_t
		stream_pause(stream_state const* state);

        /** @brief Resume an asynchronous read/write.
         *  @return stream_state.
         */
		stream_state const*
		stream_resume(size_t encrypt_handle);
		
        /** @brief Expire an asynchronous read/write. Any
         *  relative modification will be dicarded.
         */
		void
		stream_expire(size_t encrypt_handle);
        
        /** @brief Directly abort a stream_state, any change 
         *  to desitination will be dicarded.
         */
		void
		stream_abort(stream_state const* state);
        
        /** @brief Get an iterator points to the first used 
         *  address.
         *  @see AddrIterator
         */
		AddrIterator
		begin() const;

        /** @brief Get an iterator represent the one after
         *  the last used address.
         *  @see AddrIterator
         */  
		AddrIterator
		end() const;
		
        /** @brief Obtain BehaviorDB's statistic info.
         *  @see Stat
         */
		void stat(Stat * ms) const;
    
    BDBImpl *
    impl()
    { return impl_; }

	private:
		BehaviorDB(BehaviorDB const& cp);
		BehaviorDB &operator=(BehaviorDB const& cp);

		BDBImpl *impl_;
	};
	
=======

struct BDBImpl;
struct AddrIterator;

/** @brief Core class
*/
struct BDB_EXPORT BehaviorDB
{
  /** @brief Constructor
   *  @param conf
   *  @throw std::invalid_argument 
   *  @throw std::bad_alloc 
   *  @throw std::runtime_error
   *  @throw std::length_error
   *  @details Call conf.validate() internally to verify configuration.
   */
  BehaviorDB(Config const & conf);

  ~BehaviorDB();

  /** @brief Put data
   *  @param data
   *  @param size
   *  @return Address of the stored data or -1 for failure.
   */
  AddrType
  put(char const *data, size_t size);

  /** @brief Put data to a specific address
   *  @param data
   *  @param size
   *  @param addr
   *  @param off
   *  @return Address of the stored data
   *  @details If the off parameter is BDB::npos or is not given, this 
   *  method acts as an append operation.
   *  @return Address on success. -1 for failure.
   */
  AddrType
  put(char const *data, size_t size, AddrType addr, size_t off=npos);

  /** @brief std::string version put method 
  */
  AddrType
  put(std::string const& data);

  /** @brief std::string version put-to-address method 
  */
  AddrType
  put(std::string const& data, AddrType addr, size_t off=npos);

  /** @brief Replace specific address with new data
   *  @param data New data.
   *  @param size Size of new data.
   *  @param addr Address.
   *  @return Address of the new data or -1 that indicates failure.
   */
  AddrType
  update(char const* data, size_t size, AddrType addr);

  /** @brief std::string version update
  */
  AddrType
  update(std::string const& data, AddrType addr);

  /** @brief Read data from specified address and offset to a output
   *  buffer.
   *  @param output Ouput buffer.
   *  @param size Size of output buffer.
   *  @param addr Address.
   *  @param off Offset.
   *  @return Size of data read successly.
   */
  size_t
  get(char *output, size_t size, AddrType addr, size_t off=0);

  /** @brief std::string version get method
  */
  size_t
  get(std::string *output, size_t max, AddrType addr, size_t off=0);

  /** @brief Delete specified address.
   *  @return 0 for success. -1 for failure.
   */
  size_t
  del(AddrType addr);

  /** @brief Delete a segment of an address.
   *  @return Size after deleting or -1 for failure.
   */
  size_t
  del(AddrType addr, size_t off, size_t size);

  /** @brief Create output stream handle, stream_state, for 
   *  asynchronous write.
   *  @param stream_size Future size this handle will be written.
   *  @return stream_state or NULL if any error occured.
   */
  stream_state const*
  ostream(size_t stream_size);

  /** @brief Create output stream handle, stream_state, for 
   *  asynchronous write from an existed address. Offset of a
   *  chunk refered by the address can be assigned optionally.
   *  Default offset is the end of a chunk refered by the address.
   *  @return stream_state or NULL if any error occured.
   */
  stream_state const*
  ostream(size_t stream_size, AddrType addr, size_t off=npos);


  /** @brief Create output stream handle for asynchronous read.
   *  @param stream_size Size to be read.
   *  @param addr
   *  @param off
   *  @return stream_state or NULL if any error occured.
   */
  stream_state const*
  istream(size_t stream_size, AddrType addr, size_t off=0);

  /** @brief Write data to a stream_state created by ostream method.
   *  @return stream_state or NULL if any error occured.
   */
  stream_state const*
  stream_write(stream_state const* state, char const* data, size_t size);

  /** @brief Read data from a stream_state created by istream method.
   *  @return stream_state or NULL if any error occured.
   */
  stream_state const*
  stream_read(stream_state const* state, char* output, size_t size);

  /** @brief Finish asynchronous read/write. 
   *  @return Address of the operated chunk.
   */
  AddrType
  stream_finish(stream_state const* state);

  /** @brief Pause an asynchronous read/write.
   *  @return Encrypted stream state.
   *  @remark The encryption is required for
   *  preventing directly delete/free to the 
   *  stream_state.
   */
  size_t
    stream_pause(stream_state const* state);

  /** @brief Resume an asynchronous read/write.
   *  @return stream_state.
   */
  stream_state const*
    stream_resume(size_t encrypt_handle);

  /** @brief Expire an asynchronous read/write. Any
   *  relative modification will be dicarded.
   */
  void
    stream_expire(size_t encrypt_handle);

  /** @brief Directly abort a stream_state, any change 
   *  to desitination will be dicarded.
   */
  void
    stream_abort(stream_state const* state);

  /** @brief Get an iterator points to the first used 
   *  address.
   *  @see AddrIterator
   */
  AddrIterator
  begin() const;

  /** @brief Get an iterator represent the one after
   *  the last used address.
   *  @see AddrIterator
   */  
  AddrIterator
  end() const;

  /** @brief Obtain BehaviorDB's statistic info.
   *  @see Stat
   */
  void stat(Stat * ms) const;

private:
  BehaviorDB(BehaviorDB const& cp);
  BehaviorDB &operator=(BehaviorDB const& cp);

  BDBImpl *impl_;
};

>>>>>>> master
} // end of namespace BDB


/** @mainpage BehaviorDB Manual
 *  
 *  @section installation_ Installation
 *
 *  @code
 *  git clone git://github.com/yangacer/BehaviorDB.git
 *  cd BehaviorDB
 *  mkdir build
 *  cd build
 *  cmake ..
 *  ccmake . #[Optional] set build configuration
 *  make 
 *  make install clean
 *  @endcode
 *  
 *  @section quick_start_ Quick Start
 *  An example demostrates how to use BehaviorDB
 *  @include bdb.cpp
 */

#endif
