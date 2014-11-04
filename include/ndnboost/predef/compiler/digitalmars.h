/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_DIGITALMARS_H
#define NDNBOOST_PREDEF_COMPILER_DIGITALMARS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_DMC`]

[@http://en.wikipedia.org/wiki/Digital_Mars Digital Mars] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__DMC__`] [__predef_detection__]]

    [[`__DMC__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_DMC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DMC__)
#   define NDNBOOST_COMP_DMC_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRP(__DMC__)
#endif

#ifdef NDNBOOST_COMP_DMC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_DMC_EMULATED NDNBOOST_COMP_DMC_DETECTION
#   else
#       undef NDNBOOST_COMP_DMC
#       define NDNBOOST_COMP_DMC NDNBOOST_COMP_DMC_DETECTION
#   endif
#   define NDNBOOST_COMP_DMC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_DMC_NAME "Digital Mars"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DMC,NDNBOOST_COMP_DMC_NAME)

#ifdef NDNBOOST_COMP_DMC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DMC_EMULATED,NDNBOOST_COMP_DMC_NAME)
#endif


#endif
