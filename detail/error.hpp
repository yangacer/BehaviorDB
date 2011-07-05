#ifndef BDB_ERROR_HPP
#define BDB_ERROR_HPP

namespace BDB  {

	//! Error numbers
	enum ERRORNUMBER
	{
		/// Pool can not address more chunks
		ADDRESS_OVERFLOW = 1,

		/// I/O operation failure
		SYSTEM_ERROR = 2,

		/// Data is too big to be handled by BehaviorDB 
		DATA_TOO_BIG = 3,

		/// Pool is locked
		POOL_LOCKED = 4,
		
		NON_EXIST = 5
	};

	struct error_num_to_str
	{
		char const *operator()(int error_num)
		{
			return buf[error_num];
		}
	private:
		static char buf[6][40];
	};
} // end of namespace BDB

#endif // end of header
