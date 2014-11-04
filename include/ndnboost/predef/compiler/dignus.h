/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_DIGNUS_H
#define NDNBOOST_PREDEF_COMPILER_DIGNUS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_SYSC`]

[@http://www.dignus.com/dcxx/ Dignus Systems/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__SYSC__`] [__predef_detection__]]

    [[`__SYSC_VER__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_SYSC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SYSC__)
#   define NDNBOOST_COMP_SYSC_DETECTION NDNBOOST_PREDEF_MAKE_10_VRRPP(__SYSC_VER__)
#endif

#ifdef NDNBOOST_COMP_SYSC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_SYSC_EMULATED NDNBOOST_COMP_SYSC_DETECTION
#   else
#       undef NDNBOOST_COMP_SYSC
#       define NDNBOOST_COMP_SYSC NDNBOOST_COMP_SYSC_DETECTION
#   endif
#   define NDNBOOST_COMP_SYSC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_SYSC_NAME "Dignus Systems/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SYSC,NDNBOOST_COMP_SYSC_NAME)

#ifdef NDNBOOST_COMP_SYSC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_SYSC_EMULATED,NDNBOOST_COMP_SYSC_NAME)
#endif


#endif
