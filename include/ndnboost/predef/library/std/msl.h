/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_MSL_H
#define NDNBOOST_PREDEF_LIBRARY_STD_MSL_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_MSL`]

[@http://www.freescale.com/ Metrowerks] Standard C++ Library.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__MSL_CPP__`] [__predef_detection__]]
    [[`__MSL__`] [__predef_detection__]]

    [[`__MSL_CPP__`] [V.R.P]]
    [[`__MSL__`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_STD_MSL NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MSL_CPP__) || defined(__MSL__)
#   undef NDNBOOST_LIB_STD_MSL
#   if defined(__MSL_CPP__)
#       define NDNBOOST_LIB_STD_MSL NDNBOOST_PREDEF_MAKE_0X_VRPP(__MSL_CPP__)
#   else
#       define NDNBOOST_LIB_STD_MSL NDNBOOST_PREDEF_MAKE_0X_VRPP(__MSL__)
#   endif
#endif

#if NDNBOOST_LIB_STD_MSL
#   define NDNBOOST_LIB_STD_MSL_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_MSL_NAME "Metrowerks"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_MSL,NDNBOOST_LIB_STD_MSL_NAME)


#endif
