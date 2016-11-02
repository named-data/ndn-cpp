# To build using autotools, see the Development section of the file INSTALL.

AC_PREREQ([2.59])
AC_INIT([ndn-cpp], [0.11], [ndn-lib@lists.cs.ucla.edu], [ndn-cpp], [https://github.com/named-data/ndn-cpp])
AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([configure.ac])
# Create config.h without prefixes.  Put it in the include directory so CFLAGS will have -I./include .
AC_CONFIG_HEADERS(include/config.h)
# Add prefixes and output to a unique file name which will go into the ndn-cpp include install directory.
AX_PREFIX_CONFIG_H(include/ndn-cpp/ndn-cpp-config.h)
AM_MAINTAINER_MODE
AM_PROG_AR
AC_PROG_LIBTOOL
LT_PREREQ([2.2])
LT_INIT()
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_OBJCXX
AC_LANG([C++])

# Special case.
case "$host" in
  *-darwin* | *-macos10*)
     if test -d /opt/local ; then
       CFLAGS="$CFLAGS -I/opt/local/include"
       CXXFLAGS="$CXXFLAGS -I/opt/local/include"
       CPPFLAGS="$CPPFLAGS -I/opt/local/include"
       LDFLAGS="$LDFLAGS -L/opt/local/lib"
     elif test -d /sw ; then
       CFLAGS="$CFLAGS -I/sw/include"
       CXXFLAGS="$CXXFLAGS -I/sw/include"
       CPPFLAGS="$CPPFLAGS -I/sw/include"
       LDFLAGS="$LDFLAGS -L/sw/lib"
     fi
  ;;
esac

AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_FUNCS([memcmp] [memcpy] [memset])
AC_CHECK_FUNCS([strchr], :, AC_MSG_ERROR([*** strchr not found. Check 'config.log' for more details.]))
AC_CHECK_FUNCS([sscanf], :, AC_MSG_ERROR([*** sscanf not found. Check 'config.log' for more details.]))
AC_CHECK_HEADERS([time.h], :, AC_MSG_ERROR([*** time.h not found. Check 'config.log' for more details.]))
AC_CHECK_HEADERS([sys/time.h], :, AC_MSG_ERROR([*** sys/time.h not found. Check 'config.log' for more details.]))
AC_CHECK_FUNCS([gettimeofday], :, AC_MSG_ERROR([*** gettimeofday not found. processEvents requires it. Check 'config.log' for more details.]))
AC_CHECK_FUNCS([round])
AX_CXX_COMPILE_STDCXX_11(, optional)
AC_C_INLINE

# Use the minimum boost version from NFD.
AX_BOOST_BASE([1.48])

AX_BOOST_ASIO

AC_MSG_CHECKING([for std::shared_ptr])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <memory>]]
    [[std::shared_ptr<int> have_shared_ptr;]])
], [
    AC_MSG_RESULT([yes])
    HAVE_STD_SHARED_PTR=1
], [
    AC_MSG_RESULT([no])
    HAVE_STD_SHARED_PTR=0
])
AC_DEFINE_UNQUOTED([HAVE_STD_SHARED_PTR], $HAVE_STD_SHARED_PTR, [1 if have the `std::shared_ptr' class.])
AC_MSG_CHECKING([for boost::shared_ptr])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <boost/shared_ptr.hpp>]]
    [[boost::shared_ptr<int> have_shared_ptr;]])
], [
    AC_MSG_RESULT([yes])
    HAVE_BOOST_SHARED_PTR=1
], [
    AC_MSG_RESULT([no])
    HAVE_BOOST_SHARED_PTR=0
])
AC_DEFINE_UNQUOTED([HAVE_BOOST_SHARED_PTR], $HAVE_BOOST_SHARED_PTR, [1 if have the `boost::shared_ptr' class.])

# Conditionally use std::shared_ptr or boost::shared_ptr.
if test "$HAVE_STD_SHARED_PTR" == "1" ; then
  AC_ARG_WITH([std-shared-ptr],
    [AS_HELP_STRING([--with-std-shared-ptr], [ptr_lib should use std::shared_ptr, etc. if available@<:@default: yes@:>@])],
    [std_shared_ptr=${withval}], [std_shared_ptr=yes])
  if test "x${std_shared_ptr}" = xyes; then
    AC_DEFINE([WITH_STD_SHARED_PTR], 1, [Define to 1 if ptr_lib should use std::shared_ptr, etc. if available])
  else
    AC_DEFINE([WITH_STD_SHARED_PTR], 0, [Define to 1 if ptr_lib should use std::shared_ptr, etc. if available])
  fi
else
  AC_DEFINE([WITH_STD_SHARED_PTR], 0, [Define to 1 if ptr_lib should use std::shared_ptr, etc. if available])
fi
if test "$HAVE_BOOST_SHARED_PTR" == "1" ; then
  AC_ARG_WITH([boost-shared-ptr],
    [AS_HELP_STRING([--with-boost-shared-ptr], [ptr_lib should use boost::shared_ptr, etc. if available. (--with-std-shared-ptr=yes overrides this.) @<:@default: yes@:>@])],
    [boost_shared_ptr=${withval}], [boost_shared_ptr=yes])
  if test "x${boost_shared_ptr}" = xyes; then
    AC_DEFINE([WITH_BOOST_SHARED_PTR], 1, [Define to 1 if ptr_lib should use boost::shared_ptr, etc. if available])
  else
    AC_DEFINE([WITH_BOOST_SHARED_PTR], 0, [Define to 1 if ptr_lib should use boost::shared_ptr, etc. if available])
  fi
else
  AC_DEFINE([WITH_BOOST_SHARED_PTR], 0, [Define to 1 if ptr_lib should use boost::shared_ptr, etc. if available])
fi

AC_MSG_CHECKING([for std::function])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <functional>]]
    [[typedef std::function<void(int)> MyFunction;]]
    [[void myFunction(int x) {}]]
    [[MyFunction functionObject = std::bind(&myFunction, std::placeholders::_1);]])
], [
    AC_MSG_RESULT([yes])
    HAVE_STD_FUNCTION=1
], [
    AC_MSG_RESULT([no])
    HAVE_STD_FUNCTION=0
])
AC_DEFINE_UNQUOTED([HAVE_STD_FUNCTION], $HAVE_STD_FUNCTION, [1 if have the `std::function' class.])
AC_MSG_CHECKING([for boost::function])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <boost/function.hpp>]]
    [[#include <boost/bind.hpp>]]
    [[typedef boost::function<void(int)> MyFunction;]]
    [[void myFunction(int x) {}]]
    [[MyFunction functionObject = boost::bind(&myFunction, _1);]])
], [
    AC_MSG_RESULT([yes])
    HAVE_BOOST_FUNCTION=1
], [
    AC_MSG_RESULT([no])
    HAVE_BOOST_FUNCTION=0
])
AC_DEFINE_UNQUOTED([HAVE_BOOST_FUNCTION], $HAVE_BOOST_FUNCTION, [1 if have the `boost::function' class.])

# Conditionally use std::function or boost::function.
if test "$HAVE_STD_FUNCTION" == "1" ; then
  AC_ARG_WITH([std-function],
    [AS_HELP_STRING([--with-std-function], [func_lib should use std::function, etc. if available @<:@default: yes@:>@])],
    [std_function=${withval}], [std_function=yes])
  if test "x${std_function}" = xyes; then
    AC_DEFINE([WITH_STD_FUNCTION], 1, [Define to 1 if func_lib should use std::function, etc. if available])
  else
    AC_DEFINE([WITH_STD_FUNCTION], 0, [Define to 1 if func_lib should use std::function, etc. if available])
  fi
else
  AC_DEFINE([WITH_STD_FUNCTION], 0, [Define to 1 if func_lib should use std::function, etc. if available])
fi
if test "$HAVE_BOOST_FUNCTION" == "1" ; then
  AC_ARG_WITH([boost-function],
    [AS_HELP_STRING([--with-boost-function], [func_lib should use boost::function, etc. if available (--with-std-function=yes overrides this.) @<:@default: yes@:>@])],
    [boost_function=${withval}], [boost_function=yes])
  if test "x${boost_function}" = xyes; then
    AC_DEFINE([WITH_BOOST_FUNCTION], 1, [Define to 1 if func_lib should use boost::function, etc. if available])
  else
    AC_DEFINE([WITH_BOOST_FUNCTION], 0, [Define to 1 if func_lib should use boost::function, etc. if available])
  fi
else
  AC_DEFINE([WITH_BOOST_FUNCTION], 0, [Define to 1 if func_lib should use boost::function, etc. if available])
fi

AC_MSG_CHECKING([for std::regex support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <string>]]
    [[#include <regex>]]
    [[std::string result = std::regex_replace(std::string("Hello"), std::regex("H"), "h");]])
], [
    AC_MSG_RESULT([yes])
    HAVE_STD_REGEX=1
], [
    AC_MSG_RESULT([no])
    HAVE_STD_REGEX=0
])
AC_DEFINE_UNQUOTED([HAVE_STD_REGEX], $HAVE_STD_REGEX, [1 if have std::regex.])
if test "$HAVE_STD_REGEX" == "0" ; then
    # Only check for Boost regex if we don't have std regex so that we only link to its library if necessary.
    AX_BOOST_REGEX
fi

AC_MSG_CHECKING([for gmtime support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <time.h>]]
    [[#include <sys/time.h>]]
    [[struct timeval tv;]]
    [[int result1 = gettimeofday(&tv, 0);]]
    [[time_t time1 = time(0);]]
    [[struct tm* tm1 = gmtime(&time1);]]
    [[time_t time2 = timegm(tm1);]])
], [
    AC_MSG_RESULT([yes])
    AC_DEFINE_UNQUOTED([HAVE_GMTIME_SUPPORT], 1, [1 if have sys/time gmtime support including timegm.])
], [
    AC_MSG_RESULT([no])
    AC_DEFINE_UNQUOTED([HAVE_GMTIME_SUPPORT], 0, [1 if have sys/time gmtime support including timegm.])
])

# Require libcrypto.
AC_CHECK_LIB([crypto], [EVP_EncryptInit], [],
             [AC_MSG_FAILURE([can't find openssl crypto lib])])

# Conditionally use libsqlite3.  AX_LIB_SQLITE3 defines HAVE_SQLITE3 in confdefs.h .
AX_LIB_SQLITE3()
if grep -q "#define HAVE_SQLITE3" confdefs.h ; then
  AC_CHECK_LIB([sqlite3], [sqlite3_open], [],
               [AC_MSG_FAILURE([have sqlite3 headers but no libsqlite3])])
fi

# Conditionally use the OSX Security framework .
AC_MSG_CHECKING([for OSX Security framework])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <CoreFoundation/CoreFoundation.h>]]
    [[#include <Security/Security.h>]]
    [[#include <CoreServices/CoreServices.h>]]
    [[void test() { SecKeychainRef x; }]])
], [
    AC_MSG_RESULT([yes])
    HAVE_OSX_SECURITY=1
], [
    AC_MSG_RESULT([no])
    HAVE_OSX_SECURITY=0
])
AC_DEFINE_UNQUOTED([HAVE_OSX_SECURITY], $HAVE_OSX_SECURITY, [1 if have the OSX framework.])
if test "$HAVE_OSX_SECURITY" == "1" ; then
  LIBS="$LIBS -framework Security -framework Cocoa"

  AC_ARG_WITH([osx-keychain],
    [AS_HELP_STRING([--with-osx-keychain], [use the OS X Keychain as the default private key store @<:@default: yes@:>@])],
    [osx_keychain=${withval}], [osx_keychain=yes])
  if test "x${osx_keychain}" = xyes; then
    AC_DEFINE([WITH_OSX_KEYCHAIN], 1, [Define to 1 if the OS X Keychain should be the default private key store.])
  else
    AC_DEFINE([WITH_OSX_KEYCHAIN], 0, [Define to 1 if the OS X Keychain should be the default private key store.])
  fi
else
  AC_DEFINE([WITH_OSX_KEYCHAIN], 0, [Define to 1 if the OS X Keychain should be the default private key store.])
fi

AC_MSG_CHECKING([for __attribute__((deprecated))])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[void func(int); __attribute__((deprecated))]])
], [
    AC_MSG_RESULT([yes])
    AC_DEFINE_UNQUOTED([HAVE_ATTRIBUTE_DEPRECATED], 1, [1 if have `__attribute__((deprecated))'.])
], [
    AC_MSG_RESULT([no])
    AC_DEFINE_UNQUOTED([HAVE_ATTRIBUTE_DEPRECATED], 0, [1 if have `__attribute__((deprecated))'.])
])

# htonll is supported on Windows 8 but not earlier Windows.
AC_MSG_CHECKING([for htonll])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <WinSock2.h>]]
    [[htonll(1);]])
], [
    AC_MSG_RESULT([yes])
    AC_DEFINE_UNQUOTED([HAVE_HTONLL], 1, [1 if have WinSock2 `htonll'.])
], [
    AC_MSG_RESULT([no])
    AC_DEFINE_UNQUOTED([HAVE_HTONLL], 0, [1 if have WinSock2 `htonll'.])
])

AC_MSG_CHECKING([for log4cxx])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <log4cxx/logger.h>]]
    [[log4cxx::LoggerPtr staticModuleLogger;]])
], [
    AC_MSG_RESULT([yes])
    HAVE_LOG4CXX=1
], [
    AC_MSG_RESULT([no])
    HAVE_LOG4CXX=0
])
AC_DEFINE_UNQUOTED([HAVE_LOG4CXX], $HAVE_LOG4CXX, [1 if have log4cxx.])
if test "$HAVE_LOG4CXX" == "1" ; then
  AC_CHECK_LIB([log4cxx], [main], [],
               [AC_MSG_FAILURE([have log4cxx headers but no liblog4cxx])])
fi

AC_MSG_CHECKING([for Protobuf])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <google/protobuf/message.h>]]
    [[google::protobuf::Message *message;]])
], [
    AC_MSG_RESULT([yes])
    HAVE_PROTOBUF=1
], [
    AC_MSG_RESULT([no])
    HAVE_PROTOBUF=0
])
AC_DEFINE_UNQUOTED([HAVE_PROTOBUF], $HAVE_PROTOBUF, [1 if have Google Protobuf.])
if test "$HAVE_PROTOBUF" == "1" ; then
  AC_CHECK_LIB([protobuf], [main], [],
               [AC_MSG_FAILURE([have protobuf headers but no libprotobuf])])
  # libprotobuf needs libpthread.
  AC_CHECK_LIB([pthread], [main], [],
               [AC_MSG_FAILURE([have libprotobuf but no libpthread])])
fi

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([$PACKAGE_NAME],[Doxyfile])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 libndn-cpp.pc])
AC_OUTPUT
