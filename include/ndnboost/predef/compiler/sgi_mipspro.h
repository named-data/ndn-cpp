/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_SGI_MIPSPRO_H
#define NDNBOOST_PREDEF_COMPILER_SGI_MIPSPRO_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_SGI`]

[@http://en.wikipedia.org/wiki/MIPSpro SGI MIPSpro] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__sgi`] [__predef_detection__]]
    [[`sgi`] [__predef_detection__]]

    [[`_SGI_COMPILER_VERSION`] [V.R.P]]
    [[`_COMPILER_VERSION`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_SGI NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sgi) || defined(sgi)
#   if !defined(NDNBOOST_COMP_SGI_DETECTION) && defined(_SGI_COMPILER_VERSION)
#       define NDNBOOST_COMP_SGI_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(_SGI_COMPILER_VERSION)
#   endif
#   if !defined(NDNBOOST_COMP_SGI_DETECTION) && defined(_COMPILER_VERSION)
#       define NDNBOOST_COMP_SGI_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(_COMPILER_VERSION)
#   endif
#   if !defined(NDNBOOST_COMP_SGI_DETECTION)
#       define NDNBOOST_COMP_SGI_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_SGI_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_SGI_EMULATED NDNBOOST_COMP_SGI_DETECTION
#   else
#       undef NDNBOOST_COMP_SGI
#       define NDNBOOST_COMP_SGI NDNBOOST_COMP_SGI_DETECTION
#   endif
#   define NDNBOOST_COMP_SGI_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_SGI_NAME "SGI MIPSpro"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SGI,NDNBOOST_COMP_SGI_NAME)

#ifdef NDNBOOST_COMP_SGI_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SGI_EMULATED,NDNBOOST_COMP_SGI_NAME)
#endif


#endif
