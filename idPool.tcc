

template<typename B>
IDPool<B>::IDPool()
: beg_(0), end_(0), file_(0), bm_(), full_alloc_(false)
{
	// bm_.resize(4096, true);
}

template<typename B>
IDPool<B>::IDPool(char const* tfile, B beg)
: beg_(beg), end_(std::numeric_limits<B>::max()-1), file_(0), bm_(), full_alloc_(false)
{
	assert( 0 != tfile );
	assert( beg_ <= end_ );
	assert((B)-1 > end_);

	bm_.resize(4096, true);
	
	replay_transaction(tfile);
	init_transaction(tfile);
}

template<typename B>
IDPool<B>::IDPool(char const* tfile, B beg, B end)
: beg_(beg), end_(end), file_(0), bm_(), full_alloc_(true)
{
	assert(0 != tfile);
	assert( beg_ <= end_ );	
	assert((B)-1 > end_);

	bm_.resize(end_- beg_, true);
	
	replay_transaction(tfile);
	init_transaction(tfile);
}

template<typename B>
IDPool<B>::~IDPool()
{ if(*this) fclose(file_); }

template<typename B>
IDPool<B>::operator void const*() const
{
	if(!this || !file_) return 0;
	return this;
}

template<typename B>
bool 
IDPool<B>::isAcquired(B const& id) const
{ 
	if(id - beg_ >= bm_.size()) 
		return false;
	return bm_[id - beg_] == false;

}

template<typename B>
B
IDPool<B>::Acquire()
{
	assert(0 != *this);

	B rt;
	
	// extend bitmap
	if(Bitmap::npos == (rt = bm_.find_first())){
		if(!full_alloc_ ) {
			try {
				extend();  
			}catch( std::bad_alloc const &e){
				return -1;	
			}
			rt = bm_.find_first();
		}
	}
	
	if(0 > fprintf(file_, "+%lu\n", rt) && errno)
		throw std::runtime_error("IDPool(Acquire): write transaction failure");

	bm_[rt] = false;
	
	return 	beg_ + rt;
}
	
template<typename B>
int
IDPool<B>::Release(B const &id)
{
	assert(0 != *this);
	
	if(id - beg_ >= bm_.size())
		return -1;
	
	if(0 > fprintf(file_, "-%lu\n", id - beg_) && errno)
		throw std::runtime_error("IDPool(Release): write transaction failure");

	bm_[id - beg_] = true;
	return 0;
}


template<typename B>
bool 
IDPool<B>::avail() const
{ 
	assert(0 != *this);
	return bm_.any();
}

template<typename B>
B
IDPool<B>::next_used(B curID) const
{
	assert(0 != *this);
	if(curID >= end_ ) return end_;
	while(curID != end_){
		if(false == bm_[curID - beg_])
			return curID;
		++curID;	
	}
	return curID;
}

template<typename B>
size_t
IDPool<B>::size() const
{ return bm_.size(); }

template<typename B>
void 
IDPool<B>::replay_transaction(char const* transaction_file)
{
	assert(0 != transaction_file);

	assert(0 == file_ && "disallow replay when file_ has been initiated");

	FILE *tfile = fopen(transaction_file, "rb");

	if(0 == tfile) // no transaction files for replaying
		return;

	char line[21] = {0};		
	B id;
	while(fgets(line, 20, tfile)){
		line[strlen(line)-1] = 0;
		id = strtoul(&line[1], 0, 10);
		if('+' == line[0]){
			if(bm_.size() <= id){ 
				if(full_alloc_)
					throw std::runtime_error("ID in trans file does not fit into idPool");
				else	
					extend();
			}
			bm_[id] = false;
		}else if('-' == line[0]){
			bm_[id] = true;
		}
	}
	fclose(tfile);
}

template<typename B>
void 
IDPool<B>::init_transaction(char const* transaction_file)
{
	
	assert(0 != transaction_file);

	if(0 == (file_ = fopen(transaction_file,"ab")))
		throw std::runtime_error("IDPool: Fail to open transaction file");
	
	
	if(0 != setvbuf(file_, (char*)0, _IONBF, 0))
		throw std::runtime_error("IDPool: Fail to set zero buffer on transaction_file");

}

template<typename B>
void IDPool<B>::extend()
{ 
	Bitmap::size_type size = bm_.size();
	size = (size<<1) -  (size>>1);

	if( size < bm_.size() || size >= end_ - beg_)
		return;

	bm_.resize(size, true); 
}

template<typename B>
IDPool<B>::IDPool(B beg, B end)
: beg_(beg), end_(end)
{
	assert(end >= beg);
	bm_.resize(end_- beg_, true);
}

// ------------ IDValPool Impl ----------------

template<typename B, typename V>
IDValPool<B,V>::IDValPool(char const* tfile, B beg, B end)
: super(beg, end), arr_(0)
{
	arr_ = new V[end - beg];
	if(!arr_) throw std::bad_alloc();

	replay_transaction(tfile);
	super::init_transaction(tfile);
}

template<typename B, typename V>
IDValPool<B,V>::~IDValPool()
{
	delete [] arr_;	
}

template<typename B, typename V>
B IDValPool<B,V>::Acquire(V const &val)
{
	if(!*this) return -1;

	B rt;
	
	if( super::Bitmap::npos == (rt = super::bm_.find_first()) )
		return -1;
	
	if(0 > fprintf(super::file_, "+%lu\t%lu\n", rt, val) && errno)
		throw std::runtime_error("IDValPool(Acquire): write transaction failure");

	super::bm_[rt] = false;
	arr_[rt] = val;

	return 	super::beg_ + rt;
}

template<typename B, typename V>
V IDValPool<B,V>::Find(B const & id) const
{
	assert(true == isAcquired(id) && "IDValPool: Test isAcquired before Find!");
	return arr_[ id - super::beg_ ];
}


template<typename B, typename V>
void IDValPool<B,V>::Update(B const& id, V const& val)
{
	assert(true == isAcquired(id) && "IDValPool: Test isAcquired before Update!");
	
	if(0 > fprintf(super::file_, "+%lu\t%lu\n", id - super::beg_, val) && errno)
		throw std::runtime_error("IDValPool(Update): write transaction failure");
	
	arr_[id - super::beg_] = val;

}

template<typename B, typename V>
void IDValPool<B,V>::replay_transaction(char const* transaction_file)
{
	assert(0 != transaction_file);
	assert(0 == super::file_ && "disallow replay when file_ has been initiated");

	FILE *tfile = fopen(transaction_file, "rb");

	if(0 == tfile) // no transaction files for replaying
		return;
	

	char line[21] = {0};		
	B id; 
	V val;
	while(fgets(line, 20, tfile)){
		line[strlen(line)-1] = 0;
		//id = strtoul(&line[1], 0, 10);
		sscanf(line + 1, "%lu\t%lu", &id, &val);
		if('+' == line[0]){
			if(super::bm_.size() <= id)
				throw std::runtime_error("IDValPool: ID in trans file does not fit into idPool");
			super::bm_[id] = false;
			arr_[id] = val;
		}else if('-' == line[0]){
			super::bm_[id] = true;
		}
	}
	fclose(tfile);
	
}

/*
template<typename B, typename V>
size_t IDValPool<B, V>::block_size() const
{ return (super::block_size())>>20 + sizeof(V) * (super::max_size()>>20); }
*/
