/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_QNXNTO_H
#define NDNBOOST_PREDEF_OS_QNXNTO_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_QNX`]

[@http://en.wikipedia.org/wiki/QNX QNX] operating system.
Version number available as major, and minor if possible. And
version 4 is specifically detected.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__QNX__`] [__predef_detection__]]
    [[`__QNXNTO__`] [__predef_detection__]]

    [[`_NTO_VERSION`] [V.R.0]]
    [[`__QNX__`] [4.0.0]]
    ]
 */

#define NDNBOOST_OS_QNX NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__QNX__) || defined(__QNXNTO__) \
    )
#   undef NDNBOOST_OS_QNX
#   if !defined(NDNBOOST_OS_QNX) && defined(_NTO_VERSION)
#       define NDNBOOST_OS_QNX NDNBOOST_PREDEF_MAKE_10_VVRR(_NTO_VERSION)
#   endif
#   if !defined(NDNBOOST_OS_QNX) && defined(__QNX__)
#       define NDNBOOST_OS_QNX NDNBOOST_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(NDNBOOST_OS_QNX)
#       define NDNBOOST_OS_QNX NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_OS_QNX
#   define NDNBOOST_OS_QNX_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_QNX_NAME "QNX"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_QNX,NDNBOOST_OS_QNX_NAME)


#endif
