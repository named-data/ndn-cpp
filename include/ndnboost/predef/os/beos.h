/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_BEOS_H
#define NDNBOOST_PREDEF_OS_BEOS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_BEOS`]

[@http://en.wikipedia.org/wiki/BeOS BeOS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__BEOS__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_OS_BEOS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__BEOS__) \
    )
#   undef NDNBOOST_OS_BEOS
#   define NDNBOOST_OS_BEOS NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_OS_BEOS
#   define NDNBOOST_OS_BEOS_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_BEOS_NAME "BeOS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_BEOS,NDNBOOST_OS_BEOS_NAME)


#endif
