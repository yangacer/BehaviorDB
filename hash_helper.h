#ifndef _HASHHELPER_H
#define _HASHHELPER_H

#include <string>

#if defined (WIN32)
#include <hash_map>
#elif defined (__GNUC__)
#include <ext/hash_map>
#endif

namespace STL
{
#ifdef WIN32
	using stdext::hash_map;
	using stdext::hash_multimap;
#else
        using __gnu_cxx::hash_map;
	using __gnu_cxx::hash_multimap;
#endif
}

#ifndef WIN32
namespace __gnu_cxx
{
	template<>
	struct hash<std::string>{
		size_t operator()(const std::string& s) const { 
			return hash<const char*>()(s.c_str()); 
		}
	};
}
#endif

// How to use:
//
// #include <string>
// #include <iostream>
// #include "hash_helper.h"
// int main()
// {
// 	// hash map with string key and int value
//	STL::hash_map<std::string, int> hm;
//	hm["testkey"] = 1234;
//	std::cout<<hm["testkey"]<<"\n";
//	return 0;
// }

#endif // _HASHHELPER_H

