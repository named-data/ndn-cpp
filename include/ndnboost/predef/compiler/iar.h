/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_IAR_H
#define NDNBOOST_PREDEF_COMPILER_IAR_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_IAR`]

IAR C/C++ compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__IAR_SYSTEMS_ICC__`] [__predef_detection__]]

    [[`__VER__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_IAR NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IAR_SYSTEMS_ICC__)
#   define NDNBOOST_COMP_IAR_DETECTION NDNBOOST_PREDEF_MAKE_10_VVRR(__VER__)
#endif

#ifdef NDNBOOST_COMP_IAR_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_IAR_EMULATED NDNBOOST_COMP_IAR_DETECTION
#   else
#       undef NDNBOOST_COMP_IAR
#       define NDNBOOST_COMP_IAR NDNBOOST_COMP_IAR_DETECTION
#   endif
#   define NDNBOOST_COMP_IAR_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_IAR_NAME "IAR C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_IAR,NDNBOOST_COMP_IAR_NAME)

#ifdef NDNBOOST_COMP_IAR_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_IAR_EMULATED,NDNBOOST_COMP_IAR_NAME)
#endif


#endif
