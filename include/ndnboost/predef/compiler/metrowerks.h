/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_METROWERKS_H
#define NDNBOOST_PREDEF_COMPILER_METROWERKS_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_MWERKS`]

[@http://en.wikipedia.org/wiki/CodeWarrior Metrowerks CodeWarrior] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__MWERKS__`] [__predef_detection__]]
    [[`__CWCC__`] [__predef_detection__]]

    [[`__CWCC__`] [V.R.P]]
    [[`__MWERKS__`] [V.R.P >= 4.2.0]]
    [[`__MWERKS__`] [9.R.0]]
    [[`__MWERKS__`] [8.R.0]]
    ]
 */

#define NDNBOOST_COMP_MWERKS NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MWERKS__) || defined(__CWCC__)
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION) && defined(__CWCC__)
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRPP(__CWCC__)
#   endif
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x4200)
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_PREDEF_MAKE_0X_VRPP(__MWERKS__)
#   endif
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3204) // note the "skip": 04->9.3
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_VERSION_NUMBER(9,(__MWERKS__)%100-1,0)
#   endif
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3200)
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_VERSION_NUMBER(9,(__MWERKS__)%100,0)
#   endif
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3000)
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_VERSION_NUMBER(8,(__MWERKS__)%100,0)
#   endif
#   if !defined(NDNBOOST_COMP_MWERKS_DETECTION)
#       define NDNBOOST_COMP_MWERKS_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_MWERKS_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_MWERKS_EMULATED NDNBOOST_COMP_MWERKS_DETECTION
#   else
#       undef NDNBOOST_COMP_MWERKS
#       define NDNBOOST_COMP_MWERKS NDNBOOST_COMP_MWERKS_DETECTION
#   endif
#   define NDNBOOST_COMP_MWERKS_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_MWERKS_NAME "Metrowerks CodeWarrior"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MWERKS,NDNBOOST_COMP_MWERKS_NAME)

#ifdef NDNBOOST_COMP_MWERKS_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MWERKS_EMULATED,NDNBOOST_COMP_MWERKS_NAME)
#endif


#endif
