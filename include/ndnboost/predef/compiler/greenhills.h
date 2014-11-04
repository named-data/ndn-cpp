/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_GREENHILLS_H
#define NDNBOOST_PREDEF_COMPILER_GREENHILLS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_GHS`]

[@http://en.wikipedia.org/wiki/Green_Hills_Software Green Hills C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__ghs`] [__predef_detection__]]
    [[`__ghs__`] [__predef_detection__]]

    [[`__GHS_VERSION_NUMBER__`] [V.R.P]]
    [[`__ghs`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_GHS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ghs) || defined(__ghs__)
#   if !defined(NDNBOOST_COMP_GHS_DETECTION) && defined(__GHS_VERSION_NUMBER__)
#       define NDNBOOST_COMP_GHS_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(__GHS_VERSION_NUMBER__)
#   endif
#   if !defined(NDNBOOST_COMP_GHS_DETECTION) && defined(__ghs)
#       define NDNBOOST_COMP_GHS_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(__ghs)
#   endif
#   if !defined(NDNBOOST_COMP_GHS_DETECTION)
#       define NDNBOOST_COMP_GHS_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_GHS_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_GHS_EMULATED NDNBOOST_COMP_GHS_DETECTION
#   else
#       undef NDNBOOST_COMP_GHS
#       define NDNBOOST_COMP_GHS NDNBOOST_COMP_GHS_DETECTION
#   endif
#   define NDNBOOST_COMP_GHS_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_GHS_NAME "Green Hills C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GHS,NDNBOOST_COMP_GHS_NAME)

#ifdef NDNBOOST_COMP_GHS_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GHS_EMULATED,NDNBOOST_COMP_GHS_NAME)
#endif


#endif
