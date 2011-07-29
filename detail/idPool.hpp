#ifndef _IDPOOL_HPP
#define _IDPOOL_HPP

#include "boost/dynamic_bitset.hpp"
#include "common.hpp"

namespace BDB {

	/// @todo TODO: Transaction file compression (snapshot).

	/** @brief Integer ID manager within bitmap storage.
	 */

	class IDPool
	{

		friend struct bdbStater;
	protected:
		typedef AddrType BlockType;
		typedef boost::dynamic_bitset<BlockType> Bitmap;
	public:

		/** Default constructor
		 * @desc Construct a IDPool that manages numerical ID.
		 * @post An IDPool without associated transaction file.
		 * Clients can invoke init_transaction menually. Legal
		 * ID range of this IDPool is
		 * (0,  numeric_limits<BlockType>::max() - 1]. 
		 * @throw std::bad_alloc
		 */
		IDPool();
		
		/** Constructor for being given begin 
		 * @desc Construct a IDPool that manages numerical ID.
		 * @param trans_file name of a transaction file
		 * @param beg user-defined ID begin number 
		 * @pre beg < numric_limits<BlockType>::max()
		 * @post A IDPool that its storage is a partially allocated 
		 * dynamic bitmap. Legal ID range of this IDPool is
		 * (beg,  numeric_limits<BlockType>::max() - 1]. 
		 * @throw std::bad_alloc
		 */
		IDPool(char const* trans_file, AddrType beg);

		/** Constructor for being given begin and end
		 * @desc Construct a IDPool that manages numerical ID.
		 * @param trans_file name of a transaction file
		 * @param beg user-defined ID begin number 
		 * @param end user-defined ID end number 
		 * @pre beg <= end
		 * @post A IDPool that its storage is a partially allocated 
		 * dynamic bitmap. Legal ID range of this IDPool is
		 * (beg, end]. 
		 * @throw std::bad_alloc
		 */	
		IDPool(char const* trans_file, AddrType beg, AddrType end);
		
		~IDPool();

		operator void const*() const;
		
		/** Test if an ID already exists in an IDPool
		 *  @param id
		 */
		bool 
		isAcquired(AddrType const& id) const;

		/** Acquire an ID from a IDPool.
		 * @throw A write transaction error of type 
		 * std::runtime_error.
		 */
		AddrType 
		Acquire();

		/** Release an ID
		 *  @throw A write transaction error of type
		 *  std::runtime_error
		 */
		int 
		Release(AddrType const &id);

		int
		Commit(AddrType const&id);

		//bool 
		//avail() const;
		
		/** Find the first acquired ID from curID which is included
		 * @param curID Current ID
		 * @remark The curID will be tested also.
		 */
		AddrType 
		next_used(AddrType curID) const;
		
		/** The maximum count of used IDs */
		AddrType
		max_used() const;

		size_t 
		size() const;
				
		AddrType begin() const
		{ return beg_; }

		AddrType end() const
		{ return end_; }
		
		size_t
		num_blocks() const;
		
	protected:
		void 
		replay_transaction(char const* transaction_file);
		
		void 
		init_transaction(char const* transaction_file);

		int 
		write(char const *data, size_t size);
		
		/** Extend bitmap size to 1.5 times large
		 *  @throw std::bad_alloc
		 */
		void extend();

		IDPool(AddrType beg, AddrType end);

	private:
		IDPool(IDPool const &cp);
		IDPool& operator=(IDPool const &cp);
	protected:

		AddrType const beg_, end_;
		FILE*  file_;
		Bitmap bm_;
		bool full_alloc_;
		AddrType max_used_;
	};

	/** @brief Extend IDPool<B> for associating a value with an ID.
	 */
	class IDValPool : public IDPool
	{
		friend struct bdbStater;
		typedef IDPool super;
	public:
		IDValPool(char const* trans_file, AddrType beg, AddrType end);
		~IDValPool();
		
		/** Acquire an ID and associate a value with the ID
		 * @param val
		 * @return ID
		 */
		AddrType Acquire(AddrType const &val);
		
		bool avail() const;

		int Commit(AddrType const &id);

		/** Find value by ID
		 * @param id
		 * @pre id exists in an IDValPool (Test it by isAcquire())
		 * @return Associated value
		 */
		AddrType Find(AddrType const &id) const;
		
		/** Update value by ID
		 * @param id
		 * @param val
		 * @pre id exists in an IDValPool (Test it by isAcquire())
		 */
		void Update(AddrType const& id, AddrType const &val);

	protected:
		void replay_transaction(char const* transaction_file);

		// size_t block_size() const;
	private:
		AddrType* arr_;
	};
} // end of namespace BDB

#endif


