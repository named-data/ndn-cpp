/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_CLANG_H
#define NDNBOOST_PREDEF_COMPILER_CLANG_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_CLANG`]

[@http://en.wikipedia.org/wiki/Clang Clang] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__clang__`] [__predef_detection__]]

    [[`__clang_major__`, `__clang_minor__`, `__clang_patchlevel__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_CLANG NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__clang__)
#   define NDNBOOST_COMP_CLANG_DETECTION NDNBOOST_VERSION_NUMBER(__clang_major__,__clang_minor__,__clang_patchlevel__)
#endif

#ifdef NDNBOOST_COMP_CLANG_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_CLANG_EMULATED NDNBOOST_COMP_CLANG_DETECTION
#   else
#       undef NDNBOOST_COMP_CLANG
#       define NDNBOOST_COMP_CLANG NDNBOOST_COMP_CLANG_DETECTION
#   endif
#   define NDNBOOST_COMP_CLANG_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_CLANG_NAME "Clang"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_CLANG,NDNBOOST_COMP_CLANG_NAME)

#ifdef NDNBOOST_COMP_CLANG_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_CLANG_EMULATED,NDNBOOST_COMP_CLANG_NAME)
#endif


#endif
