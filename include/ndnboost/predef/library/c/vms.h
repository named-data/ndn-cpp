/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_C_VMS_H
#define NDNBOOST_PREDEF_LIBRARY_C_VMS_H

#include <ndnboost/predef/library/c/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_C_VMS`]

VMS libc Standard C library.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__CRTL_VER`] [__predef_detection__]]

    [[`__CRTL_VER`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_C_VMS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__CRTL_VER)
#   undef NDNBOOST_LIB_C_VMS
#   define NDNBOOST_LIB_C_VMS NDNBOOST_PREDEF_MAKE_10_VVRR0PP00(__CRTL_VER)
#endif

#if NDNBOOST_LIB_C_VMS
#   define NDNBOOST_LIB_C_VMS_AVAILABLE
#endif

#define NDNBOOST_LIB_C_VMS_NAME "VMS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_C_VMS,NDNBOOST_LIB_C_VMS_NAME)


#endif
