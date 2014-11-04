/*
Copyright Franz Detro 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_IOS_H
#define NDNBOOST_PREDEF_OS_IOS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_IOS`]

[@http://en.wikipedia.org/wiki/iOS iOS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__APPLE__`] [__predef_detection__]]
    [[`__MACH__`] [__predef_detection__]]
    [[`__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__`] [__predef_detection__]]

    [[`__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__`] [__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000]]
    ]
 */

#define NDNBOOST_OS_IOS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__APPLE__) && defined(__MACH__) && \
    defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
    )
#   undef NDNBOOST_OS_IOS
#   define NDNBOOST_OS_IOS (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#endif

#if NDNBOOST_OS_IOS
#   define NDNBOOST_OS_IOS_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_IOS_NAME "iOS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_IOS,NDNBOOST_OS_IOS_NAME)


#endif
