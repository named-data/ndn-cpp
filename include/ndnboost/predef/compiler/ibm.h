/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_IBM_H
#define NDNBOOST_PREDEF_COMPILER_IBM_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_IBM`]

[@http://en.wikipedia.org/wiki/VisualAge IBM XL C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__IBMCPP__`] [__predef_detection__]]
    [[`__xlC__`] [__predef_detection__]]
    [[`__xlc__`] [__predef_detection__]]

    [[`__COMPILER_VER__`] [V.R.P]]
    [[`__xlC__`] [V.R.P]]
    [[`__xlc__`] [V.R.P]]
    [[`__IBMCPP__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_IBM NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IBMCPP__) || defined(__xlC__) || defined(__xlc__)
#   if !defined(NDNBOOST_COMP_IBM_DETECTION) && defined(__COMPILER_VER__)
#       define NDNBOOST_COMP_IBM_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRRPPPP(__COMPILER_VER__)
#   endif
#   if !defined(NDNBOOST_COMP_IBM_DETECTION) && defined(__xlC__)
#       define NDNBOOST_COMP_IBM_DETECTION NDNBOOST_PREDEF_MAKE_0X_VVRR(__xlC__)
#   endif
#   if !defined(NDNBOOST_COMP_IBM_DETECTION) && defined(__xlc__)
#       define NDNBOOST_COMP_IBM_DETECTION NDNBOOST_PREDEF_MAKE_0X_VVRR(__xlc__)
#   endif
#   if !defined(NDNBOOST_COMP_IBM_DETECTION)
#       define NDNBOOST_COMP_IBM_DETECTION NDNBOOST_PREDEF_MAKE_10_VRP(__IBMCPP__)
#   endif
#endif

#ifdef NDNBOOST_COMP_IBM_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_IBM_EMULATED NDNBOOST_COMP_IBM_DETECTION
#   else
#       undef NDNBOOST_COMP_IBM
#       define NDNBOOST_COMP_IBM NDNBOOST_COMP_IBM_DETECTION
#   endif
#   define NDNBOOST_COMP_IBM_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_IBM_NAME "IBM XL C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_IBM,NDNBOOST_COMP_IBM_NAME)

#ifdef NDNBOOST_COMP_IBM_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_IBM_EMULATED,NDNBOOST_COMP_IBM_NAME)
#endif


#endif
