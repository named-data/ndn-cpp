/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_ROGUEWAVE_H
#define NDNBOOST_PREDEF_LIBRARY_STD_ROGUEWAVE_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_RW`]

[@http://stdcxx.apache.org/ Roguewave] Standard C++ library.
If available version number as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__STD_RWCOMPILER_H__`] [__predef_detection__]]
    [[`_RWSTD_VER`] [__predef_detection__]]

    [[`_RWSTD_VER`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_STD_RW NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__STD_RWCOMPILER_H__) || defined(_RWSTD_VER)
#   undef NDNBOOST_LIB_STD_RW
#   if defined(_RWSTD_VER)
#       if _RWSTD_VER < 0x010000
#           define NDNBOOST_LIB_STD_RW NDNBOOST_PREDEF_MAKE_0X_VVRRP(_RWSTD_VER)
#       else
#           define NDNBOOST_LIB_STD_RW NDNBOOST_PREDEF_MAKE_0X_VVRRPP(_RWSTD_VER)
#       endif
#   else
#       define NDNBOOST_LIB_STD_RW NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_LIB_STD_RW
#   define NDNBOOST_LIB_STD_RW_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_RW_NAME "Roguewave"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_RW,NDNBOOST_LIB_STD_RW_NAME)


#endif
