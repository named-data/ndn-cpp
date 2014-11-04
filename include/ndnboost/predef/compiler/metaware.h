/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_METAWARE_H
#define NDNBOOST_PREDEF_COMPILER_METAWARE_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_HIGHC`]

MetaWare High C/C++ compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__HIGHC__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_COMP_HIGHC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__HIGHC__)
#   define NDNBOOST_COMP_HIGHC_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#ifdef NDNBOOST_COMP_HIGHC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_HIGHC_EMULATED NDNBOOST_COMP_HIGHC_DETECTION
#   else
#       undef NDNBOOST_COMP_HIGHC
#       define NDNBOOST_COMP_HIGHC NDNBOOST_COMP_HIGHC_DETECTION
#   endif
#   define NDNBOOST_COMP_HIGHC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_HIGHC_NAME "MetaWare High C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_HIGHC,NDNBOOST_COMP_HIGHC_NAME)

#ifdef NDNBOOST_COMP_HIGHC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_HIGHC_EMULATED,NDNBOOST_COMP_HIGHC_NAME)
#endif


#endif
