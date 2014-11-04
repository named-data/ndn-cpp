/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_BORLAND_H
#define NDNBOOST_PREDEF_COMPILER_BORLAND_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_BORLAND`]

[@http://en.wikipedia.org/wiki/C_plus_plus_builder Borland C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__BORLANDC__`] [__predef_detection__]]
    [[`__CODEGEARC__`] [__predef_detection__]]

    [[`__BORLANDC__`] [V.R.P]]
    [[`__CODEGEARC__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_BORLAND NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__BORLANDC__) || defined(__CODEGEARC__)
#   if !defined(NDNBOOST_COMP_BORLAND_DETECTION) && (defined(__CODEGEARC__))
#       define NDNBOOST_COMP_BORLAND_DETECTION NDNBOOST_PREDEF_MAKE_0X_VVRP(__CODEGEARC__)
#   endif
#   if !defined(NDNBOOST_COMP_BORLAND_DETECTION)
#       define NDNBOOST_COMP_BORLAND_DETECTION NDNBOOST_PREDEF_MAKE_0X_VVRP(__BORLANDC__)
#   endif
#endif

#ifdef NDNBOOST_COMP_BORLAND_DETECTION
#   define NDNBOOST_COMP_BORLAND_AVAILABLE
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_BORLAND_EMULATED NDNBOOST_COMP_BORLAND_DETECTION
#   else
#       undef NDNBOOST_COMP_BORLAND
#       define NDNBOOST_COMP_BORLAND NDNBOOST_COMP_BORLAND_DETECTION
#   endif
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_BORLAND_NAME "Borland C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_BORLAND,NDNBOOST_COMP_BORLAND_NAME)

#ifdef NDNBOOST_COMP_BORLAND_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_BORLAND_EMULATED,NDNBOOST_COMP_BORLAND_NAME)
#endif


#endif
