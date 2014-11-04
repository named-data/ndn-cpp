/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_CYGWIN_H
#define NDNBOOST_PREDEF_OS_CYGWIN_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_CYGWIN`]

[@http://en.wikipedia.org/wiki/Cygwin Cygwin] evironment.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__CYGWIN__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_OS_CYGWIN NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__CYGWIN__) \
    )
#   undef NDNBOOST_OS_CYGWIN
#   define NDNBOOST_OS_CGYWIN NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_OS_CYGWIN
#   define NDNBOOST_OS_CYGWIN_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_CYGWIN_NAME "Cygwin"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_CYGWIN,NDNBOOST_OS_CYGWIN_NAME)


#endif
