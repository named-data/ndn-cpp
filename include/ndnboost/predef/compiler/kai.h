/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_KAI_H
#define NDNBOOST_PREDEF_COMPILER_KAI_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_KCC`]

Kai C++ compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__KCC`] [__predef_detection__]]

    [[`__KCC_VERSION`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_KCC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__KCC)
#   define NDNBOOST_COMP_KCC_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRPP(__KCC_VERSION)
#endif

#ifdef NDNBOOST_COMP_KCC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_KCC_EMULATED NDNBOOST_COMP_KCC_DETECTION
#   else
#       undef NDNBOOST_COMP_KCC
#       define NDNBOOST_COMP_KCC NDNBOOST_COMP_KCC_DETECTION
#   endif
#   define NDNBOOST_COMP_KCC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_KCC_NAME "Kai C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_KCC,NDNBOOST_COMP_KCC_NAME)

#ifdef NDNBOOST_COMP_KCC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_KCC_EMULATED,NDNBOOST_COMP_KCC_NAME)
#endif


#endif
