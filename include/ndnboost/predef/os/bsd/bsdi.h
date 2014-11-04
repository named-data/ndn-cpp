/*
Copyright Rene Rivera 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_BSD_BSDI_H
#define NDNBOOST_PREDEF_OS_BSD_BSDI_H

#include <ndnboost/predef/os/bsd.h>

/*`
[heading `NDNBOOST_OS_BSD_BSDI`]

[@http://en.wikipedia.org/wiki/BSD/OS BSDi BSD/OS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__bsdi__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_OS_BSD_BSDI NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__bsdi__) \
    )
#   ifndef NDNBOOST_OS_BSD_AVAILABLE
#       define NDNBOOST_OS_BSD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       define NDNBOOST_OS_BSD_AVAILABLE
#   endif
#   undef NDNBOOST_OS_BSD_BSDI
#   define NDNBOOST_OS_BSD_BSDI NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_OS_BSD_BSDI
#   define NDNBOOST_OS_BSD_BSDI_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_BSD_BSDI_NAME "BSDi BSD/OS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_BSD_BSDI,NDNBOOST_OS_BSD_BSDI_NAME)

#endif
