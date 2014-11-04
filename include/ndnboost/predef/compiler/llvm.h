/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_LLVM_H
#define NDNBOOST_PREDEF_COMPILER_LLVM_H

/* Other compilers that emulate this one need to be detected first. */

#include <ndnboost/predef/compiler/clang.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_LLVM`]

[@http://en.wikipedia.org/wiki/LLVM LLVM] compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__llvm__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_COMP_LLVM NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__llvm__)
#   define NDNBOOST_COMP_LLVM_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#ifdef NDNBOOST_COMP_LLVM_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_LLVM_EMULATED NDNBOOST_COMP_LLVM_DETECTION
#   else
#       undef NDNBOOST_COMP_LLVM
#       define NDNBOOST_COMP_LLVM NDNBOOST_COMP_LLVM_DETECTION
#   endif
#   define NDNBOOST_COMP_LLVM_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_LLVM_NAME "LLVM"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_LLVM,NDNBOOST_COMP_LLVM_NAME)

#ifdef NDNBOOST_COMP_LLVM_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_LLVM_EMULATED,NDNBOOST_COMP_LLVM_NAME)
#endif


#endif
