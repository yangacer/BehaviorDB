#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#ifdef __GNUC__ // GNU

#elif defined(_WIN32) // MSVC

// ftello/fseeko
#define ftello(X) _ftelli64(X)
#define fseeko(X,Y,Z) _fseeki64(X,Y,Z)

#endif

namespace BDB {
	typedef unsigned int AddrType;
} // end of nemaespace BDB

#endif // end of header
