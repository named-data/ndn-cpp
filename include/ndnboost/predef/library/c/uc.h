/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_C_UC_H
#define NDNBOOST_PREDEF_LIBRARY_C_UC_H

#include <ndnboost/predef/library/c/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_C_UC`]

[@http://en.wikipedia.org/wiki/Uclibc uClibc] Standard C library.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__UCLIBC__`] [__predef_detection__]]

    [[`__UCLIBC_MAJOR__`, `__UCLIBC_MINOR__`, `__UCLIBC_SUBLEVEL__`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_C_UC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__UCLIBC__)
#   undef NDNBOOST_LIB_C_UC
#   define NDNBOOST_LIB_C_UC NDNBOOST_VERSION_NUMBER(\
        __UCLIBC_MAJOR__,__UCLIBC_MINOR__,__UCLIBC_SUBLEVEL__)
#endif

#if NDNBOOST_LIB_C_UC
#   define NDNBOOST_LIB_C_UC_AVAILABLE
#endif

#define NDNBOOST_LIB_C_UC_NAME "uClibc"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_C_UC,NDNBOOST_LIB_C_UC_NAME)


#endif
