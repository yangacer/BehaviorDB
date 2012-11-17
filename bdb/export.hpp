// Usage: Use cmake to generate export header 
// e.g. 
// File: CMakeLists.txt
// configure_file( export.hpp.in /data2/BehaviorDB/export.hpp )
//

#ifndef BDB_EXPORT_HPP_
#define BDB_EXPORT_HPP_

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define BDB_HELPER_DLL_IMPORT __declspec(dllimport)
  #define BDB_HELPER_DLL_EXPORT __declspec(dllexport)
  #define BDB_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define BDB_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define BDB_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define BDB_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define BDB_HELPER_DLL_IMPORT
    #define BDB_HELPER_DLL_EXPORT
    #define BDB_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define BDB_API and BDB_LOCAL.
// BDB_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// BDB_LOCAL is used for non-api symbols.

#ifdef BDB_DLL // defined if BDB is compiled as a DLL
  #ifdef BDB_DLL_EXPORTS // defined if we are building the BDB DLL (instead of using it)
    #define BDB_API BDB_HELPER_DLL_EXPORT
  #else
    #define BDB_API BDB_HELPER_DLL_IMPORT
  #endif // BDB_DLL_EXPORTS
  #define BDB_LOCAL BDB_HELPER_DLL_LOCAL
#else // BDB_DLL is not defined: this means BDB is a static lib.
  #define BDB_API
  #define BDB_LOCAL
#endif // BDB_DLL

#endif
