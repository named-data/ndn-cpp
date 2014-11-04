/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_DINKUMWARE_H
#define NDNBOOST_PREDEF_LIBRARY_STD_DINKUMWARE_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_DINKUMWARE`]

[@http://en.wikipedia.org/wiki/Dinkumware Dinkumware] Standard C++ Library.
If available version number as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_YVALS`, `__IBMCPP__`] [__predef_detection__]]
    [[`_CPPLIB_VER`] [__predef_detection__]]

    [[`_CPPLIB_VER`] [V.R.0]]
    ]
 */

#define NDNBOOST_LIB_STD_DINKUMWARE NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if (defined(_YVALS) && !defined(__IBMCPP__)) || defined(_CPPLIB_VER)
#   undef NDNBOOST_LIB_STD_DINKUMWARE
#   if defined(_CPPLIB_VER)
#       define NDNBOOST_LIB_STD_DINKUMWARE NDNBOOST_PREDEF_MAKE_10_VVRR(_CPPLIB_VER)
#   else
#       define NDNBOOST_LIB_STD_DINKUMWARE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_LIB_STD_DINKUMWARE
#   define NDNBOOST_LIB_STD_DINKUMWARE_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_DINKUMWARE_NAME "Dinkumware"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_DINKUMWARE,NDNBOOST_LIB_STD_DINKUMWARE_NAME)


#endif
