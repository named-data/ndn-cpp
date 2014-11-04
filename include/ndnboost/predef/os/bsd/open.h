/*
Copyright Rene Rivera 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_BSD_OPEN_H
#define NDNBOOST_PREDEF_OS_BSD_OPEN_H

#include <ndnboost/predef/os/bsd.h>

/*`
[heading `NDNBOOST_OS_BSD_OPEN`]

[@http://en.wikipedia.org/wiki/Openbsd OpenBSD] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__OpenBSD__`] [__predef_detection__]]

    [[`OpenBSD2_0`] [2.0.0]]
    [[`OpenBSD2_1`] [2.1.0]]
    [[`OpenBSD2_2`] [2.2.0]]
    [[`OpenBSD2_3`] [2.3.0]]
    [[`OpenBSD2_4`] [2.4.0]]
    [[`OpenBSD2_5`] [2.5.0]]
    [[`OpenBSD2_6`] [2.6.0]]
    [[`OpenBSD2_7`] [2.7.0]]
    [[`OpenBSD2_8`] [2.8.0]]
    [[`OpenBSD2_9`] [2.9.0]]
    [[`OpenBSD3_0`] [3.0.0]]
    [[`OpenBSD3_1`] [3.1.0]]
    [[`OpenBSD3_2`] [3.2.0]]
    [[`OpenBSD3_3`] [3.3.0]]
    [[`OpenBSD3_4`] [3.4.0]]
    [[`OpenBSD3_5`] [3.5.0]]
    [[`OpenBSD3_6`] [3.6.0]]
    [[`OpenBSD3_7`] [3.7.0]]
    [[`OpenBSD3_8`] [3.8.0]]
    [[`OpenBSD3_9`] [3.9.0]]
    [[`OpenBSD4_0`] [4.0.0]]
    [[`OpenBSD4_1`] [4.1.0]]
    [[`OpenBSD4_2`] [4.2.0]]
    [[`OpenBSD4_3`] [4.3.0]]
    [[`OpenBSD4_4`] [4.4.0]]
    [[`OpenBSD4_5`] [4.5.0]]
    [[`OpenBSD4_6`] [4.6.0]]
    [[`OpenBSD4_7`] [4.7.0]]
    [[`OpenBSD4_8`] [4.8.0]]
    [[`OpenBSD4_9`] [4.9.0]]
    ]
 */

#define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__OpenBSD__) \
    )
#   ifndef NDNBOOST_OS_BSD_AVAILABLE
#       define NDNBOOST_OS_BSD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       define NDNBOOST_OS_BSD_AVAILABLE
#   endif
#   undef NDNBOOST_OS_BSD_OPEN
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_0)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_1)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,1,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_2)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,2,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_3)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,3,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_4)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,4,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_5)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,5,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_6)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,6,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_7)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,7,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_8)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,8,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD2_9)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(2,9,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_0)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_1)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,1,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_2)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_3)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,3,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_4)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,4,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_5)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,5,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_6)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,6,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_7)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,7,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_8)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,8,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD3_9)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(3,9,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_0)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_1)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,1,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_2)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,2,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_3)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,3,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_4)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,4,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_5)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,5,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_6)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,6,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_7)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,7,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_8)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,8,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN) && defined(OpenBSD4_9)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER(4,9,0)
#   endif
#   if !defined(NDNBOOST_OS_BSD_OPEN)
#       define NDNBOOST_OS_BSD_OPEN NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_OS_BSD_OPEN
#   define NDNBOOST_OS_BSD_OPEN_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_BSD_OPEN_NAME "OpenBSD"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_BSD_OPEN,NDNBOOST_OS_BSD_OPEN_NAME)

#endif
