/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_MPW_H
#define NDNBOOST_PREDEF_COMPILER_MPW_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_MPW`]

[@http://en.wikipedia.org/wiki/Macintosh_Programmer%27s_Workshop MPW C++] compiler.
Version number available as major, and minor.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__MRC__`] [__predef_detection__]]
    [[`MPW_C`] [__predef_detection__]]
    [[`MPW_CPLUS`] [__predef_detection__]]

    [[`__MRC__`] [V.R.0]]
    ]
 */

#define NDNBOOST_COMP_MPW NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MRC__) || defined(MPW_C) || defined(MPW_CPLUS)
#   if !defined(NDNBOOST_COMP_MPW_DETECTION) && defined(__MRC__)
#       define NDNBOOST_COMP_MPW_DETECTION NDNBOOST_PREDEF_MAKE_0X_VVRR(__MRC__)
#   endif
#   if !defined(NDNBOOST_COMP_MPW_DETECTION)
#       define NDNBOOST_COMP_MPW_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_MPW_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_MPW_EMULATED NDNBOOST_COMP_MPW_DETECTION
#   else
#       undef NDNBOOST_COMP_MPW
#       define NDNBOOST_COMP_MPW NDNBOOST_COMP_MPW_DETECTION
#   endif
#   define NDNBOOST_COMP_MPW_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_MPW_NAME "MPW C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MPW,NDNBOOST_COMP_MPW_NAME)

#ifdef NDNBOOST_COMP_MPW_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MPW_EMULATED,NDNBOOST_COMP_MPW_NAME)
#endif


#endif
