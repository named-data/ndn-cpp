/*
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_OS400_H
#define NDNBOOST_PREDEF_OS_OS400_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_OS400`]

[@http://en.wikipedia.org/wiki/IBM_i IBM OS/400] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__OS400__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_OS_OS400 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__OS400__) \
    )
#   undef NDNBOOST_OS_OS400
#   define NDNBOOST_OS_OS400 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_OS_OS400
#   define NDNBOOST_OS_OS400_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_OS400_NAME "IBM OS/400"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_OS400,NDNBOOST_OS_OS400_NAME)


#endif
