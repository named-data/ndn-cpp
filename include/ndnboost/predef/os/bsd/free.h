/*
Copyright Rene Rivera 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_BSD_FREE_H
#define NDNBOOST_PREDEF_OS_BSD_FREE_H

#include <ndnboost/predef/os/bsd.h>

/*`
[heading `NDNBOOST_OS_BSD_FREE`]

[@http://en.wikipedia.org/wiki/Freebsd FreeBSD] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__FreeBSD__`] [__predef_detection__]]

    [[`__FreeBSD_version`] [V.R.P]]
    ]
 */

#define NDNBOOST_OS_BSD_FREE NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__FreeBSD__) \
    )
#   ifndef NDNBOOST_OS_BSD_AVAILABLE
#       define NDNBOOST_OS_BSD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       define NDNBOOST_OS_BSD_AVAILABLE
#   endif
#   undef NDNBOOST_OS_BSD_FREE
#   if defined(__FreeBSD_version)
#       if __FreeBSD_version < 500000
#           define NDNBOOST_OS_BSD_FREE \
                NDNBOOST_PREDEF_MAKE_10_VRP000(__FreeBSD_version)
#       else
#           define NDNBOOST_OS_BSD_FREE \
                NDNBOOST_PREDEF_MAKE_10_VRR000(__FreeBSD_version)
#       endif
#   else
#       define NDNBOOST_OS_BSD_FREE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_OS_BSD_FREE
#   define NDNBOOST_OS_BSD_FREE_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_BSD_FREE_NAME "Free BSD"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_BSD_FREE,NDNBOOST_OS_BSD_FREE_NAME)

#endif
