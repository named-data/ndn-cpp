/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_WINDOWS_H
#define NDNBOOST_PREDEF_OS_WINDOWS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_WINDOWS`]

[@http://en.wikipedia.org/wiki/Category:Microsoft_Windows Microsoft Windows] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_WIN32`] [__predef_detection__]]
    [[`_WIN64`] [__predef_detection__]]
    [[`__WIN32__`] [__predef_detection__]]
    [[`__TOS_WIN__`] [__predef_detection__]]
    [[`__WINDOWS__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_OS_WINDOWS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__) \
    )
#   undef NDNBOOST_OS_WINDOWS
#   define NDNBOOST_OS_WINDOWS NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_OS_WINDOWS
#   define NDNBOOST_OS_WINDOWS_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_WINDOWS_NAME "Microsoft Windows"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_WINDOWS,NDNBOOST_OS_WINDOWS_NAME)

#endif
