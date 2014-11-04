/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_EDG_H
#define NDNBOOST_PREDEF_COMPILER_EDG_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_EDG`]

[@http://en.wikipedia.org/wiki/Edison_Design_Group EDG C++ Frontend] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__EDG__`] [__predef_detection__]]

    [[`__EDG_VERSION__`] [V.R.0]]
    ]
 */

#define NDNBOOST_COMP_EDG NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__EDG__)
#   define NDNBOOST_COMP_EDG_DETECTION NDNBOOST_PREDEF_MAKE_10_VRR(__EDG_VERSION__)
#endif

#ifdef NDNBOOST_COMP_EDG_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_EDG_EMULATED NDNBOOST_COMP_EDG_DETECTION
#   else
#       undef NDNBOOST_COMP_EDG
#       define NDNBOOST_COMP_EDG NDNBOOST_COMP_EDG_DETECTION
#   endif
#   define NDNBOOST_COMP_EDG_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_EDG_NAME "EDG C++ Frontend"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_EDG,NDNBOOST_COMP_EDG_NAME)

#ifdef NDNBOOST_COMP_EDG_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_EDG_EMULATED,NDNBOOST_COMP_EDG_NAME)
#endif


#endif
