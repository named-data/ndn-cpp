/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_SGI_H
#define NDNBOOST_PREDEF_LIBRARY_STD_SGI_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_SGI`]

[@http://www.sgi.com/tech/stl/ SGI] Standard C++ library.
If available version number as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__STL_CONFIG_H`] [__predef_detection__]]

    [[`__SGI_STL`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_STD_SGI NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__STL_CONFIG_H)
#   undef NDNBOOST_LIB_STD_SGI
#   if defined(__SGI_STL)
#       define NDNBOOST_LIB_STD_SGI NDNBOOST_PREDEF_MAKE_0X_VRP(__SGI_STL)
#   else
#       define NDNBOOST_LIB_STD_SGI NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_LIB_STD_SGI
#   define NDNBOOST_LIB_STD_SGI_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_SGI_NAME "SGI"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_SGI,NDNBOOST_LIB_STD_SGI_NAME)


#endif
