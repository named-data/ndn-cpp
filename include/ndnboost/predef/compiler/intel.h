/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_INTEL_H
#define NDNBOOST_PREDEF_COMPILER_INTEL_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_INTEL`]

[@http://en.wikipedia.org/wiki/Intel_C%2B%2B Intel C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__INTEL_COMPILER`] [__predef_detection__]]
    [[`__ICL`] [__predef_detection__]]
    [[`__ICC`] [__predef_detection__]]
    [[`__ECC`] [__predef_detection__]]

    [[`__INTEL_COMPILER`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_INTEL NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || \
    defined(__ECC)
#   if !defined(NDNBOOST_COMP_INTEL_DETECTION) && defined(__INTEL_COMPILER)
#       define NDNBOOST_COMP_INTEL_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(__INTEL_COMPILER)
#   endif
#   if !defined(NDNBOOST_COMP_INTEL_DETECTION)
#       define NDNBOOST_COMP_INTEL_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_INTEL_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_INTEL_EMULATED NDNBOOST_COMP_INTEL_DETECTION
#   else
#       undef NDNBOOST_COMP_INTEL
#       define NDNBOOST_COMP_INTEL NDNBOOST_COMP_INTEL_DETECTION
#   endif
#   define NDNBOOST_COMP_INTEL_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_INTEL_NAME "Intel C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_INTEL,NDNBOOST_COMP_INTEL_NAME)

#ifdef NDNBOOST_COMP_INTEL_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_INTEL_EMULATED,NDNBOOST_COMP_INTEL_NAME)
#endif


#endif
