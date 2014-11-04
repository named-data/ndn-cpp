/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_DIAB_H
#define NDNBOOST_PREDEF_COMPILER_DIAB_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_DIAB`]

[@http://www.windriver.com/products/development_suite/wind_river_compiler/ Diab C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__DCC__`] [__predef_detection__]]

    [[`__VERSION_NUMBER__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_DIAB NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DCC__)
#   define NDNBOOST_COMP_DIAB_DETECTION NDNBOOST_PREDEF_MAKE_10_VRPP(__VERSION_NUMBER__)
#endif

#ifdef NDNBOOST_COMP_DIAB_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_DIAB_EMULATED NDNBOOST_COMP_DIAB_DETECTION
#   else
#       undef NDNBOOST_COMP_DIAB
#       define NDNBOOST_COMP_DIAB NDNBOOST_COMP_DIAB_DETECTION
#   endif
#   define NDNBOOST_COMP_DIAB_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_DIAB_NAME "Diab C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DIAB,NDNBOOST_COMP_DIAB_NAME)

#ifdef NDNBOOST_COMP_DIAB_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DIAB_EMULATED,NDNBOOST_COMP_DIAB_NAME)
#endif


#endif
