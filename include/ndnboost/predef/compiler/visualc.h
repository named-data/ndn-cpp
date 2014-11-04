/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_VISUALC_H
#define NDNBOOST_PREDEF_COMPILER_VISUALC_H

/* Other compilers that emulate this one need to be detected first. */

#include <ndnboost/predef/compiler/clang.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_MSVC`]

[@http://en.wikipedia.org/wiki/Visual_studio Microsoft Visual C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_MSC_VER`] [__predef_detection__]]

    [[`_MSC_FULL_VER`] [V.R.P]]
    [[`_MSC_VER`] [V.R.0]]
    ]
 */

#define NDNBOOST_COMP_MSVC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_MSC_VER)
#   if !defined (_MSC_FULL_VER)
#       define NDNBOOST_COMP_MSVC_BUILD 0
#   else
        /* how many digits does the build number have? */
#       if _MSC_FULL_VER / 10000 == _MSC_VER
            /* four digits */
#           define NDNBOOST_COMP_MSVC_BUILD (_MSC_FULL_VER % 10000)
#       elif _MSC_FULL_VER / 100000 == _MSC_VER
            /* five digits */
#           define NDNBOOST_COMP_MSVC_BUILD (_MSC_FULL_VER % 100000)
#       else
#           error "Cannot determine build number from _MSC_FULL_VER"
#       endif
#   endif
#   define NDNBOOST_COMP_MSVC_DETECTION NDNBOOST_VERSION_NUMBER(\
        _MSC_VER/100-6,\
        _MSC_VER%100,\
        NDNBOOST_COMP_MSVC_BUILD)
#endif

#ifdef NDNBOOST_COMP_MSVC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_MSVC_EMULATED NDNBOOST_COMP_MSVC_DETECTION
#   else
#       undef NDNBOOST_COMP_MSVC
#       define NDNBOOST_COMP_MSVC NDNBOOST_COMP_MSVC_DETECTION
#   endif
#   define NDNBOOST_COMP_MSVC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_MSVC_NAME "Microsoft Visual C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MSVC,NDNBOOST_COMP_MSVC_NAME)

#ifdef NDNBOOST_COMP_MSVC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MSVC_EMULATED,NDNBOOST_COMP_MSVC_NAME)
#endif


#endif
