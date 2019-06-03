#ifndef _INCLUDE_NDN_CPP_NDN_CPP_CONFIG_H
#define _INCLUDE_NDN_CPP_NDN_CPP_CONFIG_H 1
 
/* include/ndn-cpp/ndn-cpp-config.h. Generated automatically at end of configure. */
/* include/config.h.  Generated from config.h.in by configure.  */
/* include/config.h.in.  Generated from configure.ac by autoheader.  */

/* 1 if have `__attribute__((deprecated))'. */
#ifndef NDN_CPP_HAVE_ATTRIBUTE_DEPRECATED
#define NDN_CPP_HAVE_ATTRIBUTE_DEPRECATED 1
#endif

/* define if the Boost library is available */
//#define NDN_CPP_HAVE_BOOST /**/

/* define if the Boost::ASIO library is available */
//#define NDN_CPP_HAVE_BOOST_ASIO /**/

/* 1 if have the `boost::function' class. */
#ifndef NDN_CPP_HAVE_BOOST_FUNCTION
#define NDN_CPP_HAVE_BOOST_FUNCTION 0
#endif

/* define if the Boost::Regex library is available */
/* #undef HAVE_BOOST_REGEX */

/* 1 if have the `boost::shared_ptr' class. */
#ifndef NDN_CPP_HAVE_BOOST_SHARED_PTR
#define NDN_CPP_HAVE_BOOST_SHARED_PTR 0
#endif

/* define if the compiler supports basic C++11 syntax */
#ifndef NDN_CPP_HAVE_CXX11
#define NDN_CPP_HAVE_CXX11 0
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef NDN_CPP_HAVE_DLFCN_H
#define NDN_CPP_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef NDN_CPP_HAVE_GETTIMEOFDAY
#define NDN_CPP_HAVE_GETTIMEOFDAY 1
#endif

/* 1 if have sys/time gmtime support including timegm. */
#ifndef NDN_CPP_HAVE_GMTIME_SUPPORT
#define NDN_CPP_HAVE_GMTIME_SUPPORT 1
#endif

/* 1 if have WinSock2 `htonll'. */
#ifndef NDN_CPP_HAVE_HTONLL
#define NDN_CPP_HAVE_HTONLL 0
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef NDN_CPP_HAVE_INTTYPES_H
#define NDN_CPP_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#ifndef NDN_CPP_HAVE_LIBCRYPTO
#define NDN_CPP_HAVE_LIBCRYPTO 1
#endif

/* Define to 1 if you have the `log4cxx' library (-llog4cxx). */
/* #undef HAVE_LIBLOG4CXX */

/* Define to 1 if you have the `protobuf' library (-lprotobuf). */
#ifndef NDN_CPP_HAVE_LIBPROTOBUF
#define NDN_CPP_HAVE_LIBPROTOBUF 0
#endif

/* Define to 1 if you have the `pthread' library (-lpthread). */
#ifndef NDN_CPP_HAVE_LIBPTHREAD
#define NDN_CPP_HAVE_LIBPTHREAD 1
#endif

/* Define to 1 if you have the `sqlite3' library (-lsqlite3). */
#ifndef NDN_CPP_HAVE_LIBSQLITE3
#define NDN_CPP_HAVE_LIBSQLITE3 0
#endif

/* Define to 1 if you have `z' library (-lz) */
#ifndef NDN_CPP_HAVE_LIBZ
#define NDN_CPP_HAVE_LIBZ 0
#endif

/* 1 if have log4cxx. */
#ifndef NDN_CPP_HAVE_LOG4CXX
#define NDN_CPP_HAVE_LOG4CXX 0
#endif

/* Define to 1 if you have the `memcmp' function. */
#ifndef NDN_CPP_HAVE_MEMCMP
#define NDN_CPP_HAVE_MEMCMP 1
#endif

/* Define to 1 if you have the `memcpy' function. */
#ifndef NDN_CPP_HAVE_MEMCPY
#define NDN_CPP_HAVE_MEMCPY 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef NDN_CPP_HAVE_MEMORY_H
#define NDN_CPP_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#ifndef NDN_CPP_HAVE_MEMSET
#define NDN_CPP_HAVE_MEMSET 1
#endif

/* 1 if have the OSX framework. */
#ifndef NDN_CPP_HAVE_OSX_SECURITY
#define NDN_CPP_HAVE_OSX_SECURITY 0
#endif

/* 1 if have Google Protobuf. */
#ifndef NDN_CPP_HAVE_PROTOBUF
#define NDN_CPP_HAVE_PROTOBUF 0
#endif

/* Define to 1 if you have the `round' function. */
#ifndef NDN_CPP_HAVE_ROUND
#define NDN_CPP_HAVE_ROUND 1
#endif

/* Have the SQLITE3 library */
#define NDN_CPP_HAVE_SQLITE3

/* Define to 1 if you have the `sscanf' function. */
#ifndef NDN_CPP_HAVE_SSCANF
#define NDN_CPP_HAVE_SSCANF 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef NDN_CPP_HAVE_STDINT_H
#define NDN_CPP_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef NDN_CPP_HAVE_STDLIB_H
#define NDN_CPP_HAVE_STDLIB_H 1
#endif

/* 1 if have the `std::function' class. */
#ifndef NDN_CPP_HAVE_STD_FUNCTION
#define NDN_CPP_HAVE_STD_FUNCTION 1
#endif

/* 1 if have std::regex. */
#ifndef NDN_CPP_HAVE_STD_REGEX
#define NDN_CPP_HAVE_STD_REGEX 1
#endif

/* 1 if have the `std::shared_ptr' class. */
#ifndef NDN_CPP_HAVE_STD_SHARED_PTR
#define NDN_CPP_HAVE_STD_SHARED_PTR 1
#endif

/* Define to 1 if you have the `strchr' function. */
#ifndef NDN_CPP_HAVE_STRCHR
#define NDN_CPP_HAVE_STRCHR 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef NDN_CPP_HAVE_STRINGS_H
#define NDN_CPP_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef NDN_CPP_HAVE_STRING_H
#define NDN_CPP_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef NDN_CPP_HAVE_SYS_STAT_H
#define NDN_CPP_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef NDN_CPP_HAVE_SYS_TIME_H
#define NDN_CPP_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef NDN_CPP_HAVE_SYS_TYPES_H
#define NDN_CPP_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <time.h> header file. */
#ifndef NDN_CPP_HAVE_TIME_H
#define NDN_CPP_HAVE_TIME_H 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef NDN_CPP_HAVE_UNISTD_H
#define NDN_CPP_HAVE_UNISTD_H 1
#endif

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#ifndef NDN_CPP_LT_OBJDIR
#define NDN_CPP_LT_OBJDIR ".libs/"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef NDN_CPP_PACKAGE_BUGREPORT
#define NDN_CPP_PACKAGE_BUGREPORT "ndn-lib@lists.cs.ucla.edu"
#endif

/* Define to the full name of this package. */
#ifndef NDN_CPP_PACKAGE_NAME
#define NDN_CPP_PACKAGE_NAME "ndn-cpp"
#endif

/* Define to the full name and version of this package. */
#ifndef NDN_CPP_PACKAGE_STRING
#define NDN_CPP_PACKAGE_STRING "ndn-cpp 0.16"
#endif

/* Define to the one symbol short name of this package. */
#ifndef NDN_CPP_PACKAGE_TARNAME
#define NDN_CPP_PACKAGE_TARNAME "ndn-cpp"
#endif

/* Define to the home page for this package. */
#ifndef NDN_CPP_PACKAGE_URL
#define NDN_CPP_PACKAGE_URL "https://github.com/named-data/ndn-cpp"
#endif

/* Define to the version of this package. */
#ifndef NDN_CPP_PACKAGE_VERSION
#define NDN_CPP_PACKAGE_VERSION "0.16"
#endif

/* Define to 1 if you have the ANSI C header files. */
#ifndef NDN_CPP_STDC_HEADERS
#define NDN_CPP_STDC_HEADERS 1
#endif

/* Define to 1 if func_lib should use boost::function, etc. if available */
#ifndef NDN_CPP_WITH_BOOST_FUNCTION
#define NDN_CPP_WITH_BOOST_FUNCTION 0
#endif

/* Define to 1 if ptr_lib should use boost::shared_ptr, etc. if available */
#ifndef NDN_CPP_WITH_BOOST_SHARED_PTR
#define NDN_CPP_WITH_BOOST_SHARED_PTR 0
#endif

/* Define to 1 if the OS X Keychain should be the default private key store.
   */
#ifndef NDN_CPP_WITH_OSX_KEYCHAIN
#define NDN_CPP_WITH_OSX_KEYCHAIN 0
#endif

/* Define to 1 if func_lib should use std::function, etc. if available */
#ifndef NDN_CPP_WITH_STD_FUNCTION
#define NDN_CPP_WITH_STD_FUNCTION 1
#endif

/* Define to 1 if ptr_lib should use std::shared_ptr, etc. if available */
#ifndef NDN_CPP_WITH_STD_SHARED_PTR
#define NDN_CPP_WITH_STD_SHARED_PTR 1
#endif

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */
 
/* once: _INCLUDE_NDN_CPP_NDN_CPP_CONFIG_H */
#endif
