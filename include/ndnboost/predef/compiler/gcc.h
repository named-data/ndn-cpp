/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_GCC_H
#define NDNBOOST_PREDEF_COMPILER_GCC_H

/* Other compilers that emulate this one need to be detected first. */

#include <ndnboost/predef/compiler/clang.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_GNUC`]

[@http://en.wikipedia.org/wiki/GNU_Compiler_Collection Gnu GCC C/C++] compiler.
Version number available as major, minor, and patch (if available).

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__GNUC__`] [__predef_detection__]]

    [[`__GNUC__`, `__GNUC_MINOR__`, `__GNUC_PATCHLEVEL__`] [V.R.P]]
    [[`__GNUC__`, `__GNUC_MINOR__`] [V.R.0]]
    ]
 */

#define NDNBOOST_COMP_GNUC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GNUC__)
#   if !defined(NDNBOOST_COMP_GNUC_DETECTION) && defined(__GNUC_PATCHLEVEL__)
#       define NDNBOOST_COMP_GNUC_DETECTION \
            NDNBOOST_VERSION_NUMBER(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#   endif
#   if !defined(NDNBOOST_COMP_GNUC_DETECTION)
#       define NDNBOOST_COMP_GNUC_DETECTION \
            NDNBOOST_VERSION_NUMBER(__GNUC__,__GNUC_MINOR__,0)
#   endif
#endif

#ifdef NDNBOOST_COMP_GNUC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_GNUC_EMULATED NDNBOOST_COMP_GNUC_DETECTION
#   else
#       undef NDNBOOST_COMP_GNUC
#       define NDNBOOST_COMP_GNUC NDNBOOST_COMP_GNUC_DETECTION
#   endif
#   define NDNBOOST_COMP_GNUC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_GNUC_NAME "Gnu GCC C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GNUC,NDNBOOST_COMP_GNUC_NAME)

#ifdef NDNBOOST_COMP_GNUC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GNUC_EMULATED,NDNBOOST_COMP_GNUC_NAME)
#endif


#endif
