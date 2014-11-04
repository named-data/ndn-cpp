/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_PALM_H
#define NDNBOOST_PREDEF_COMPILER_PALM_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_PALM`]

Palm C/C++ compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_PACC_VER`] [__predef_detection__]]

    [[`_PACC_VER`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_PALM NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_PACC_VER)
#   define NDNBOOST_COMP_PALM_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRRPP000(_PACC_VER)
#endif

#ifdef NDNBOOST_COMP_PALM_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_PALM_EMULATED NDNBOOST_COMP_PALM_DETECTION
#   else
#       undef NDNBOOST_COMP_PALM
#       define NDNBOOST_COMP_PALM NDNBOOST_COMP_PALM_DETECTION
#   endif
#   define NDNBOOST_COMP_PALM_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_PALM_NAME "Palm C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PALM,NDNBOOST_COMP_PALM_NAME)

#ifdef NDNBOOST_COMP_PALM_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PALM_EMULATED,NDNBOOST_COMP_PALM_NAME)
#endif


#endif
