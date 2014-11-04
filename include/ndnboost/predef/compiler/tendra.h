/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_TENDRA_H
#define NDNBOOST_PREDEF_COMPILER_TENDRA_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_TENDRA`]

[@http://en.wikipedia.org/wiki/TenDRA_Compiler TenDRA C/C++] compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__TenDRA__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_COMP_TENDRA NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__TenDRA__)
#   define NDNBOOST_COMP_TENDRA_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#ifdef NDNBOOST_COMP_TENDRA_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_TENDRA_EMULATED NDNBOOST_COMP_TENDRA_DETECTION
#   else
#       undef NDNBOOST_COMP_TENDRA
#       define NDNBOOST_COMP_TENDRA NDNBOOST_COMP_TENDRA_DETECTION
#   endif
#   define NDNBOOST_COMP_TENDRA_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_TENDRA_NAME "TenDRA C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_TENDRA,NDNBOOST_COMP_TENDRA_NAME)

#ifdef NDNBOOST_COMP_TENDRA_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_TENDRA_EMULATED,NDNBOOST_COMP_TENDRA_NAME)
#endif


#endif
