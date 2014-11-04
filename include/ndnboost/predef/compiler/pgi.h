/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_PGI_H
#define NDNBOOST_PREDEF_COMPILER_PGI_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_PGI`]

[@http://en.wikipedia.org/wiki/The_Portland_Group Portland Group C/C++] compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__PGI`] [__predef_detection__]]

    [[`__PGIC__`, `__PGIC_MINOR__`, `__PGIC_PATCHLEVEL__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_PGI NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__PGI)
#   if !defined(NDNBOOST_COMP_PGI_DETECTION) && (defined(__PGIC__) && defined(__PGIC_MINOR__) && defined(__PGIC_PATCHLEVEL__))
#       define NDNBOOST_COMP_PGI_DETECTION NDNBOOST_VERSION_NUMBER(__PGIC__,__PGIC_MINOR__,__PGIC_PATCHLEVEL__)
#   endif
#   if !defined(NDNBOOST_COMP_PGI_DETECTION)
#       define NDNBOOST_COMP_PGI_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_PGI_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_PGI_EMULATED NDNBOOST_COMP_PGI_DETECTION
#   else
#       undef NDNBOOST_COMP_PGI
#       define NDNBOOST_COMP_PGI NDNBOOST_COMP_PGI_DETECTION
#   endif
#   define NDNBOOST_COMP_PGI_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_PGI_NAME "Portland Group C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PGI,NDNBOOST_COMP_PGI_NAME)

#ifdef NDNBOOST_COMP_PGI_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PGI_EMULATED,NDNBOOST_COMP_PGI_NAME)
#endif


#endif
