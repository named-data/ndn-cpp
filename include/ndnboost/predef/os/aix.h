/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_AIX_H
#define NDNBOOST_PREDEF_OS_AIX_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_AIX`]

[@http://en.wikipedia.org/wiki/AIX_operating_system IBM AIX] operating system.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_AIX`] [__predef_detection__]]
    [[`__TOS_AIX__`] [__predef_detection__]]

    [[`_AIX43`] [4.3.0]]
    [[`_AIX41`] [4.1.0]]
    [[`_AIX32`] [3.2.0]]
    [[`_AIX3`] [3.0.0]]
    ]
 */

#define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(_AIX) || defined(__TOS_AIX__) \
    )
#   undef NDNBOOST_OS_AIX
#   if !defined(NDNBOOST_OS_AIX) && defined(_AIX43)
#       define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER(4,3,0)
#   endif
#   if !defined(NDNBOOST_OS_AIX) && defined(_AIX41)
#       define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER(4,1,0)
#   endif
#   if !defined(NDNBOOST_OS_AIX) && defined(_AIX32)
#       define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(NDNBOOST_OS_AIX) && defined(_AIX3)
#       define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(NDNBOOST_OS_AIX)
#       define NDNBOOST_OS_AIX NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_OS_AIX
#   define NDNBOOST_OS_AIX_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_AIX_NAME "IBM AIX"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_AIX,NDNBOOST_OS_AIX_NAME)


#endif
