/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_WATCOM_H
#define NDNBOOST_PREDEF_COMPILER_WATCOM_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_WATCOM`]

[@http://en.wikipedia.org/wiki/Watcom Watcom C++] compiler.
Version number available as major, and minor.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__WATCOMC__`] [__predef_detection__]]

    [[`__WATCOMC__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_WATCOM NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__WATCOMC__)
#   define NDNBOOST_COMP_WATCOM_DETECTION NDNBOOST_PREDEF_MAKE_10_VVRR(__WATCOMC__)
#endif

#ifdef NDNBOOST_COMP_WATCOM_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_WATCOM_EMULATED NDNBOOST_COMP_WATCOM_DETECTION
#   else
#       undef NDNBOOST_COMP_WATCOM
#       define NDNBOOST_COMP_WATCOM NDNBOOST_COMP_WATCOM_DETECTION
#   endif
#   define NDNBOOST_COMP_WATCOM_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_WATCOM_NAME "Watcom C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_WATCOM,NDNBOOST_COMP_WATCOM_NAME)

#ifdef NDNBOOST_COMP_WATCOM_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_WATCOM_EMULATED,NDNBOOST_COMP_WATCOM_NAME)
#endif


#endif
