/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_COMEAU_H
#define NDNBOOST_PREDEF_COMPILER_COMEAU_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

#define NDNBOOST_COMP_COMO NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

/*`
[heading `NDNBOOST_COMP_COMO`]

[@http://en.wikipedia.org/wiki/Comeau_C/C%2B%2B Comeau C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__COMO__`] [__predef_detection__]]

    [[`__COMO_VERSION__`] [V.R.P]]
    ]
 */

#if defined(__COMO__)
#   if !defined(NDNBOOST_COMP_COMO_DETECTION) && defined(__CONO_VERSION__)
#       define NDNBOOST_COMP_COMO_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRP(__COMO_VERSION__)
#   endif
#   if !defined(NDNBOOST_COMP_COMO_DETECTION)
#       define NDNBOOST_COMP_COMO_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_COMO_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_COMO_EMULATED NDNBOOST_COMP_COMO_DETECTION
#   else
#       undef NDNBOOST_COMP_COMO
#       define NDNBOOST_COMP_COMO NDNBOOST_COMP_COMO_DETECTION
#   endif
#   define NDNBOOST_COMP_COMO_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_COMO_NAME "Comeau C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_COMO,NDNBOOST_COMP_COMO_NAME)

#ifdef NDNBOOST_COMP_COMO_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_COMO_EMULATED,NDNBOOST_COMP_COMO_NAME)
#endif


#endif
