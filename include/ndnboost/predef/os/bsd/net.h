/*
Copyright Rene Rivera 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_BSD_NET_H
#define NDNBOOST_PREDEF_OS_BSD_NET_H

#include <ndnboost/predef/os/bsd.h>

/*`
[heading `NDNBOOST_OS_BSD_NET`]

[@http://en.wikipedia.org/wiki/Netbsd NetBSD] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__NETBSD__`] [__predef_detection__]]
    [[`__NetBSD__`] [__predef_detection__]]

    [[`__NETBSD_version`] [V.R.P]]
    [[`NetBSD0_8`] [0.8.0]]
    [[`NetBSD0_9`] [0.9.0]]
    [[`NetBSD1_0`] [1.0.0]]
    [[`__NetBSD_Version`] [V.R.P]]
    ]
 */

#define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__NETBSD__) || defined(__NetBSD__) \
    )
#   ifndef NDNBOOST_OS_BSD_AVAILABLE
#       define NDNBOOST_OS_BSD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       define NDNBOOST_OS_BSD_AVAILABLE
#   endif
#   undef NDNBOOST_OS_BSD_NET
#   if defined(__NETBSD__)
#       if defined(__NETBSD_version)
#           if __NETBSD_version < 500000
#               define NDNBOOST_OS_BSD_NET \
                    NDNBOOST_PREDEF_MAKE_10_VRP000(__NETBSD_version)
#           else
#               define NDNBOOST_OS_BSD_NET \
                    NDNBOOST_PREDEF_MAKE_10_VRR000(__NETBSD_version)
#           endif
#       else
#           define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   elif defined(__NetBSD__)
#       if !defined(NDNBOOST_OS_BSD_NET) && defined(NetBSD0_8)
#           define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER(0,8,0)
#       endif
#       if !defined(NDNBOOST_OS_BSD_NET) && defined(NetBSD0_9)
#           define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER(0,9,0)
#       endif
#       if !defined(NDNBOOST_OS_BSD_NET) && defined(NetBSD1_0)
#           define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER(1,0,0)
#       endif
#       if !defined(NDNBOOST_OS_BSD_NET) && defined(__NetBSD_Version)
#           define NDNBOOST_OS_BSD_NET \
                NDNBOOST_PREDEF_MAKE_10_VVRR00PP00(__NetBSD_Version)
#       endif
#       if !defined(NDNBOOST_OS_BSD_NET)
#           define NDNBOOST_OS_BSD_NET NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

#if NDNBOOST_OS_BSD_NET
#   define NDNBOOST_OS_BSD_NET_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_BSD_NET_NAME "DragonFly BSD"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_BSD_NET,NDNBOOST_OS_BSD_NET_NAME)

#endif
