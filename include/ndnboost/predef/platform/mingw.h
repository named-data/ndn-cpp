/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_MINGW_H
#define NDNBOOST_PREDEF_COMPILER_MINGW_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_PLAT_MINGW`]

[@http://en.wikipedia.org/wiki/MinGW MinGW] platform.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__MINGW32__`] [__predef_detection__]]
    [[`__MINGW64__`] [__predef_detection__]]

    [[`__MINGW64_VERSION_MAJOR`, `__MINGW64_VERSION_MINOR`] [V.R.0]]
    [[`__MINGW32_VERSION_MAJOR`, `__MINGW32_VERSION_MINOR`] [V.R.0]]
    ]
 */

#define NDNBOOST_PLAT_MINGW NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MINGW32__) || defined(__MINGW64__)
#   include <_mingw.h>
#   if !defined(NDNBOOST_PLAT_MINGW_DETECTION) && (defined(__MINGW64_VERSION_MAJOR) && defined(__MINGW64_VERSION_MINOR))
#       define NDNBOOST_PLAT_MINGW_DETECTION \
            NDNBOOST_VERSION_NUMBER(__MINGW64_VERSION_MAJOR,__MINGW64_VERSION_MINOR,0)
#   endif
#   if !defined(NDNBOOST_PLAT_MINGW_DETECTION) && (defined(__MINGW32_VERSION_MAJOR) && defined(__MINGW32_VERSION_MINOR))
#       define NDNBOOST_PLAT_MINGW_DETECTION \
            NDNBOOST_VERSION_NUMBER(__MINGW32_MAJOR_VERSION,__MINGW32_MINOR_VERSION,0)
#   endif
#   if !defined(NDNBOOST_PLAT_MINGW_DETECTION)
#       define NDNBOOST_PLAT_MINGW_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_PLAT_MINGW_DETECTION
#   define NDNBOOST_PLAT_MINGW_AVAILABLE
#   if defined(NDNBOOST_PREDEF_DETAIL_PLAT_DETECTED)
#       define NDNBOOST_PLAT_MINGW_EMULATED NDNBOOST_PLAT_MINGW_DETECTION
#   else
#       undef NDNBOOST_PLAT_MINGW
#       define NDNBOOST_PLAT_MINGW NDNBOOST_PLAT_MINGW_DETECTION
#   endif
#   include <ndnboost/predef/detail/platform_detected.h>
#endif

#define NDNBOOST_PLAT_MINGW_NAME "MinGW"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_PLAT_MINGW,NDNBOOST_PLAT_MINGW_NAME)

#ifdef NDNBOOST_PLAT_MINGW_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_PLAT_MINGW_EMULATED,NDNBOOST_PLAT_MINGW_NAME)
#endif


#endif
