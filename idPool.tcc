

template<typename B>
IDPool<B>::IDPool()
: beg_(0), end_(std::numeric_limits<B>::max()-1), file_(0), bm_(), full_alloc_(false)
{
	bm_.resize(4096, true);
}

template<typename B>
IDPool<B>::IDPool(B beg)
: beg_(beg), end_(std::numeric_limits<B>::max()-1), file_(0), bm_(), full_alloc_(false)
{
	assert( beg_ <= end_ );
	bm_.resize(4096, true);
}

template<typename B>
IDPool<B>::IDPool(B beg, B end)
: beg_(beg), end_(end), file_(0), bm_(), full_alloc_(true)
{
	assert( beg_ <= end_ );	
	bm_.resize(end_- beg_, true);
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
	if(!*this) return -1;

	Bitmap::size_type rt;
	
	// extend bitmap
	if(Bitmap::npos == (rt = bm_.find_first())){
		if(!full_alloc_ && 
			!extend() && 
			Bitmap::npos == (rt = bm_.find_first()) )
		{
			return -1;
		}
	}
	
	if(0 > fprintf(file_, "+%lu\n", rt) && errno){
		fprintf(stderr, "idPool: %s\n", strerror(errno));
		exit(1);
	}

	bm_[rt] = false;
	
	return 	beg_ + rt;
}
	
template<typename B>
int
IDPool<B>::Release(B const &id)
{
	if(!*this) return -1;

	if(id - beg_ >= bm_.size())
		return -1;
	
	if(0 > fprintf(file_, "-%lu\n", id - beg_) && errno){
		fprintf(stderr,"%s\n", strerror(errno));
		exit(1);
	}

	bm_[id - beg_] = true;
	return 0;
}


template<typename B>
bool 
IDPool<B>::avail() const
{ 
	if(!*this) return false;
	return bm_.any(); 
}

template<typename B>
size_t
IDPool<B>::size() const
{ return bm_.size(); }

template<typename B>
size_t
IDPool<B>::max_size() const
{ 
	if(full_alloc_)	
		return end_ - beg_; 
	return bm_.max_size() - beg_;
}

template<typename B>
void 
IDPool<B>::replay_transaction(char const* transaction_file)
{
	FILE *tfile = fopen(transaction_file, "rb");

	if(0 == tfile){ // no transaction files for replaying
		//fprintf(stderr, "No transaction replay at %s\n", transaction_file);
		errno = 0;
		return;
	}
	

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
IDPool<B>::init_transaction(char const* transaction_file) throw(std::runtime_error)
{
	
	if(0 == (file_ = fopen(transaction_file,"ab"))){
		fprintf(stderr, "Fail to open %s; system(%s)\n", transaction_file, strerror(errno));
		throw std::runtime_error("Fail to open transaction file");
	}
	
	if(0 != setvbuf(file_, (char*)0, _IONBF, 0)){
		fprintf(stderr, "Fail to set zero buffer on %s;system(%s)\n", transaction_file, strerror(errno));
		throw std::runtime_error("Fail to set zero buffer on transaction_file");
	}
	

}

template<typename B>
bool IDPool<B>::extend()
{ 
	Bitmap::size_type size = bm_.size();
	size = (size<<1) -  (size>>1);
	if( size < bm_.size() || size >= end_ - beg_){
		bm_.resize(end_ - beg_, true);
		return false;
	}
	bm_.resize(size, true); 
	return true;
}


// ------------ IDValPool Impl ----------------

template<typename B, typename V>
IDValPool<B,V>::IDValPool(B beg, B end)
: super(beg, end), arr_(0)
{
	arr_ = new V[end - beg];
	if(!arr_) throw std::bad_alloc();
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

	typename super::Bitmap::size_type rt;
	
	// extend bitmap
	if(super::Bitmap::npos == (rt = super::bm_.find_first())){
		return -1;
	}
	
	if(0 > fprintf(super::file_, "+%lu\t%lu\n", rt, val) && errno){
		fprintf(stderr, "idPool: %s\n", strerror(errno));
		exit(1);
	}

	super::bm_[rt] = false;
	arr_[rt] = val;

	return 	super::beg_ + rt;


}

template<typename B, typename V>
V IDValPool<B,V>::Find(B const & id) const
{
	return isAcquired(id) ? 
		arr_[ id - super::beg_ ] : 
		-1;
}


template<typename B, typename V>
void IDValPool<B,V>::Update(B const& id, V const& val)
{
	if(!isAcquired(id)) return;
	
	if(0 > fprintf(super::file_, "+%lu\t%lu\n", id - super::beg_, val) && errno){
		fprintf(stderr, "idPool: %s\n", strerror(errno));
		exit(1);
	}
	
	arr_[id - super::beg_] = val;

}

template<typename B, typename V>
void IDValPool<B,V>::replay_transaction(char const* transaction_file)
{
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
				throw std::runtime_error("ID in trans file does not fit into idPool");
			super::bm_[id] = false;
			arr_[id] = val;
		}else if('-' == line[0]){
			super::bm_[id] = true;
		}
	}
	fclose(tfile);
	
}


