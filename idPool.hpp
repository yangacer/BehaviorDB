#ifndef _IDPOOL_HPP
#define _IDPOOL_HPP

#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include "boost/dynamic_bitset.hpp"

/// @todo TODO: Transaction file compression.
template<typename BlockType = unsigned int>
class IDPool
{
protected:
	typedef boost::dynamic_bitset<> Bitmap;
public:
	// Default CTOR
	// beg: 0 
	// end: numeric_limits<BlockType>::max()
	// storage: dynamic bitmap
	// partial allocation
	IDPool();
	
	// beg: user-defined (<= numric_limits<BlockType>::max())
	// end: numric_limits<BlockType>::max()
	// storage: dynamic bitmap
	// partial allocation
	IDPool(BlockType beg);

	// beg: user-defined (<= numric_limits<BlockType>::max())
	// end: user-defined (<= numric_limits<BlockType>::max())
	IDPool(BlockType beg, BlockType end);
	
	~IDPool();

	operator void const*() const;
	
	bool isAcquired(BlockType const& id) const;

	BlockType Acquire();
	
	int Release(BlockType const &id);

	bool avail() const;
	
	size_t size() const;
	
	size_t max_size() const;
	
	void replay_transaction(char const* transaction_file);
	
	void init_transaction(char const* transaction_file) throw(std::runtime_error);
	
protected:
	
	bool extend();
	
private:
	IDPool(IDPool const &cp);
	IDPool& operator=(IDPool const &cp);
protected:

	BlockType const beg_, end_;
	FILE*  file_;
	Bitmap bm_;
	bool full_alloc_;

};

template<typename BlockType, typename ValueType>
class IDValPool : public IDPool<BlockType>
{
	typedef IDPool<BlockType> super;
public:
	IDValPool(BlockType beg, BlockType end);
	~IDValPool();

	BlockType Acquire(ValueType const &val);

	void replay_transaction(char const* transaction_file);
private:
	ValueType* arr_;
};

#include "idPool.tcc"

#endif

