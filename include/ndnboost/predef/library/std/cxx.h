/*
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_CXX_H
#define NDNBOOST_PREDEF_LIBRARY_STD_CXX_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_CXX`]

[@http://libcxx.llvm.org/ libc++] C++ Standard Library.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_LIBCPP_VERSION`] [__predef_detection__]]

    [[`_LIBCPP_VERSION`] [V.0.P]]
    ]
 */

#define NDNBOOST_LIB_STD_CXX NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_LIBCPP_VERSION)
#   undef NDNBOOST_LIB_STD_CXX
#   define NDNBOOST_LIB_STD_CXX NDNBOOST_PREDEF_MAKE_10_VPPP(_LIBCPP_VERSION)
#endif

#if NDNBOOST_LIB_STD_CXX
#   define NDNBOOST_LIB_STD_CXX_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_CXX_NAME "libc++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_CXX,NDNBOOST_LIB_STD_CXX_NAME)


#endif
