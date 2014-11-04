/*
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_OS_VMS_H
#define NDNBOOST_PREDEF_OS_VMS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_OS_VMS`]

[@http://en.wikipedia.org/wiki/Vms VMS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`VMS`] [__predef_detection__]]
    [[`__VMS`] [__predef_detection__]]

    [[`__VMS_VER`] [V.R.P]]
    ]
 */

#define NDNBOOST_OS_VMS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(NDNBOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(VMS) || defined(__VMS) \
    )
#   undef NDNBOOST_OS_VMS
#   if defined(__VMS_VER)
#       define NDNBOOST_OS_VMS NDNBOOST_PREDEF_MAKE_10_VVRR00PP00(__VMS_VER)
#   else
#       define NDNBOOST_OS_VMS NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_OS_VMS
#   define NDNBOOST_OS_VMS_AVAILABLE
#   include <ndnboost/predef/detail/os_detected.h>
#endif

#define NDNBOOST_OS_VMS_NAME "VMS"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_OS_VMS,NDNBOOST_OS_VMS_NAME)


#endif
