#ifndef FIXEDPOOL_HPP_
#define FIXEDPOOL_HPP_

#include "common.hpp"
#include <string>
#include <cstdio>
#include <cstring>

namespace BDB {
	
	template<typename T, size_t Size>
	struct fixed_pool
	{
		typedef T value_type;
		
		fixed_pool() 
		: id_(0), work_dir_(""), file_(0)
		{}

		fixed_pool(unsigned int id, char const* work_dir)
		: id_(0), work_dir_(""), file_(0)
		{
			open(id, work_dir);
		}
		
		~fixed_pool()
		{
			if(file_) fclose(file_);	
		}
		
		operator void const *() const
		{ 	
			if(!file_) return 0;
			return this;
		}

		void
		open(unsigned int id, char const* work_dir)
		{
			id_ = id;
			work_dir_ = work_dir;

			char fname[256];
			if(work_dir_.size() > 256) {
				fprintf(stderr, "length of pool_dir string is too long\n");
				exit(1);
			}
			
			sprintf(fname, "%s%04x.fpo", work_dir_.c_str(), id_);
			if(0 == (file_ = fopen(fname, "r+b"))){
				if(0 == (file_ = fopen(fname, "w+b"))){
					fprintf(stderr, "create fix pool failed\n");
					exit(1);
				}
			}
			setvbuf(file_, 0, _IONBF, 0);
			

		}

		int read(T* val, AddrType addr) const
		{
			if(!*this) return -1;

			off_t loc_addr = addr;
			loc_addr *= Size;
			if(-1 == fseeko(file_, loc_addr, SEEK_SET))
				return -1;
			file_>>*val;
			if(ferror(file_)) return -1;
			return 0;
		}

		int write(T const & val, AddrType addr)
		{
			if(!*this) return -1;

			off_t loc_addr = addr;
			loc_addr *= Size;
			if(-1 == fseeko(file_, loc_addr, SEEK_SET))
				return -1;
			file_<<val;
			if(ferror(file_)) return -1;
			return 0;
		}
		
		std::string
		dir() const 
		{
			return work_dir_;	
		}
	private:
		unsigned int id_;
		std::string work_dir_;
		FILE* file_;		
	};
}

#endif // end of header
