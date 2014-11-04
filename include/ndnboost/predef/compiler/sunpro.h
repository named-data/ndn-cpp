/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_SUNPRO_H
#define NDNBOOST_PREDEF_COMPILER_SUNPRO_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_SUNPRO`]

[@http://en.wikipedia.org/wiki/Sun_Studio_%28software%29 Sun Studio] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__SUNPRO_CC`] [__predef_detection__]]
    [[`__SUNPRO_C`] [__predef_detection__]]

    [[`__SUNPRO_CC`] [V.R.P]]
    [[`__SUNPRO_C`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_SUNPRO NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#   if !defined(NDNBOOST_COMP_SUNPRO_DETECTION) && defined(__SUNPRO_CC)
#       define NDNBOOST_COMP_SUNPRO_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRP(__SUNPRO_CC)
#   endif
#   if !defined(NDNBOOST_COMP_SUNPRO_DETECTION) && defined(__SUNPRO_C)
#       define NDNBOOST_COMP_SUNPRO_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRP(__SUNPRO_C)
#   endif
#   if !defined(NDNBOOST_COMP_SUNPRO_DETECTION)
#       define NDNBOOST_COMP_SUNPRO_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_SUNPRO_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_SUNPRO_EMULATED NDNBOOST_COMP_SUNPRO_DETECTION
#   else
#       undef NDNBOOST_COMP_SUNPRO
#       define NDNBOOST_COMP_SUNPRO NDNBOOST_COMP_SUNPRO_DETECTION
#   endif
#   define NDNBOOST_COMP_SUNPRO_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_SUNPRO_NAME "Sun Studio"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SUNPRO,NDNBOOST_COMP_SUNPRO_NAME)

#ifdef NDNBOOST_COMP_SUNPRO_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SUNPRO_EMULATED,NDNBOOST_COMP_SUNPRO_NAME)
#endif


#endif
