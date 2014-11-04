/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_C_ZOS_H
#define NDNBOOST_PREDEF_LIBRARY_C_ZOS_H

#include <ndnboost/predef/library/c/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_C_ZOS`]

z/OS libc Standard C library.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__LIBREL__`] [__predef_detection__]]

    [[`__LIBREL__`] [V.R.P]]
    [[`__TARGET_LIB__`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_C_ZOS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__LIBREL__)
#   undef NDNBOOST_LIB_C_ZOS
#   if !defined(NDNBOOST_LIB_C_ZOS) && defined(__LIBREL__)
#       define NDNBOOST_LIB_C_ZOS NDNBOOST_PREDEF_MAKE_0X_VRRPPPP(__LIBREL__)
#   endif
#   if !defined(NDNBOOST_LIB_C_ZOS) && defined(__TARGET_LIB__)
#       define NDNBOOST_LIB_C_ZOS NDNBOOST_PREDEF_MAKE_0X_VRRPPPP(__TARGET_LIB__)
#   endif
#   if !defined(NDNBOOST_LIB_C_ZOS)
#       define NDNBOOST_LIB_C_ZOS NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_LIB_C_ZOS
#   define NDNBOOST_LIB_C_ZOS_AVAILABLE
#endif

#define NDNBOOST_LIB_C_ZOS_NAME "z/OS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_C_ZOS,NDNBOOST_LIB_C_ZOS_NAME)


#endif
